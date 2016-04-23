#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "common/common.h"
#include "common/fs_defs.h"
#include "common/loader_defs.h"
#include "game/rpx_rpl_table.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/os_functions.h"
#include "kernel/kernel_functions.h"
#include "function_hooks.h"
#include "discdumper.h"

#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000

#define USE_EXTRA_LOG_FUNCTIONS   0

#define DECL(res, name, ...) \
        res (* real_ ## name)(__VA_ARGS__) __attribute__((section(".data"))); \
        res my_ ## name(__VA_ARGS__)

static u32 gLoaderPhysicalBufferAddr __attribute__((section(".data"))) = 0;

DECL(int, FSBindMount, void *pClient, void *pCmd, char *source, char *target, int error)
{
    if(strcmp(target, "/vol/app_priv") == 0 && IsDumpingDiscUsbMeta())
    {
        //! on game discs
        //! redirect mount target path to /vol/meta and dump it then unmount and re-mount on the target position
        char acPath[10];
        strcpy(acPath, "/vol/meta");

        int res = real_FSBindMount(pClient, pCmd, source, acPath, error);
        if(res == 0)
        {
            DumpMetaPath(pClient, pCmd, NULL);
            FSBindUnmount(pClient, pCmd, acPath, -1);
        }
    }
    return real_FSBindMount(pClient, pCmd, source, target, error);
}

DECL(int, OSDynLoad_Acquire, char* rpl, unsigned int *handle, int r5 __attribute__((unused))) {
    int result = real_OSDynLoad_Acquire(rpl, handle, 0);

    if(rpxRplTableGetCount() > 0)
    {
        DumpRpxRpl(NULL);
    }

    return result;
}

// This function is called every time after LiBounceOneChunk.
// It waits for the asynchronous call of LiLoadAsync for the IOSU to fill data to the RPX/RPL address
// and return the still remaining bytes to load.
// We override it and replace the loaded date from LiLoadAsync with our data and our remaining bytes to load.
DECL(int, LiWaitOneChunk, int * iRemainingBytes, const char *filename, int fileType)
{
    int result;
    int remaining_bytes = 0;
    unsigned int core_id;

    int *sgBufferNumber;
    int *sgBounceError;
    int *sgGotBytes;
    int *sgTotalBytes;
    int *sgIsLoadingBuffer;
    int *sgFinishedLoadingBuffer;
    unsigned int * __load_reply;

    // get the offset of per core global variable for dynload initialized (just a simple address + (core_id * 4))
    unsigned int gDynloadInitialized;

    // get the current core
    asm volatile("mfspr %0, 0x3EF" : "=r" (core_id));

    // Comment (Dimok):
    // time measurement at this position for logger  -> we don't need it right now except maybe for debugging
    //unsigned long long systemTime1 = Loader_GetSystemTime();

	if(OS_FIRMWARE == 550)
    {
        // pointer to global variables of the loader
        loader_globals_550_t *loader_globals = (loader_globals_550_t*)(0xEFE19E80);

        gDynloadInitialized = *(volatile unsigned int*)(0xEFE13DBC + (core_id << 2));
        __load_reply = (unsigned int *)0xEFE1D998;
        sgBufferNumber = &loader_globals->sgBufferNumber;
        sgBounceError = &loader_globals->sgBounceError;
        sgGotBytes = &loader_globals->sgGotBytes;
        sgTotalBytes = &loader_globals->sgTotalBytes;
        sgFinishedLoadingBuffer = &loader_globals->sgFinishedLoadingBuffer;
        // not available on 5.5.x
        sgIsLoadingBuffer = NULL;
    }
    else
    {
        // pointer to global variables of the loader
        loader_globals_t *loader_globals = (loader_globals_t*)(0xEFE19D00);

        gDynloadInitialized = *(volatile unsigned int*)(0xEFE13C3C + (core_id << 2));
        __load_reply = (unsigned int *)0xEFE1D818;
        sgBufferNumber = &loader_globals->sgBufferNumber;
        sgBounceError = &loader_globals->sgBounceError;
        sgGotBytes = &loader_globals->sgGotBytes;
        sgIsLoadingBuffer = &loader_globals->sgIsLoadingBuffer;
        // not available on < 5.5.x
        sgTotalBytes = NULL;
        sgFinishedLoadingBuffer = NULL;
    }

    // the data loading was started in LiBounceOneChunk() and here it waits for IOSU to finish copy the data
    if(gDynloadInitialized != 0) {
        result = LiWaitIopCompleteWithInterrupts(0x2160EC0, &remaining_bytes);

    }
    else {
        result = LiWaitIopComplete(0x2160EC0, &remaining_bytes);
    }


    // Comment (Dimok):
    // time measurement at this position for logger -> we don't need it right now except maybe for debugging
    //unsigned long long systemTime2 = Loader_GetSystemTime();

    //------------------------------------------------------------------------------------------------------------------
    // Start of our function intrusion:
    // After IOSU is done writing the data into the 0xF6000000/0xF6400000 address,
    // we overwrite it with our data before setting the global flag for IsLoadingBuffer to 0
    // Do this only if we are in the game that was launched by our method
    if((result == 0) && *(volatile unsigned int*)0xEFE00000 != 0x6d656e2e && *(volatile unsigned int*)0xEFE00000 != 0x66666C5F)
    {
        s_rpx_rpl *rpl_struct = rpxRplTableGet();
        int found = 0;
        int entryIndex = rpxRplTableGetCount();

        while(entryIndex > 0 && rpl_struct)
        {
            // if we load RPX then the filename can't be checked as it is the Mii Maker or Smash Bros RPX name
            // there skip the filename check for RPX
            int len = strlen(filename);
            int len2 = strlen(rpl_struct->name);
            if ((len != len2) && (len != (len2 - 4)))
            {
                rpl_struct = rpl_struct->next;
                continue;
            }

            if(strncasecmp(filename, rpl_struct->name, len) == 0)
            {
                found = 1;
                break;
            }

            rpl_struct = rpl_struct->next;
        }

        unsigned int load_address = (*sgBufferNumber == 1) ? gLoaderPhysicalBufferAddr : (gLoaderPhysicalBufferAddr + 0x00400000); // virtual 0xF6000000 and 0xF6400000

        int bytes_loaded = remaining_bytes;
        if(remaining_bytes == 0)
        {
            bytes_loaded = __load_reply[3];
        }
        else
        {
            bytes_loaded = remaining_bytes;
        }

        int system_rpl = 0;

        //! all RPL seem to have some kind of header of 0x80 bytes except the system ones
        if(!found && fileType == 1)
        {
            if(*(volatile unsigned int *)(load_address + 0x80) == 0x7F454C46)
            {
                load_address += 0x80;
                bytes_loaded -= 0x80;
                system_rpl = 0;
            }
            else
                system_rpl = 1;
        }

        if(!system_rpl)
        {
            if(!found)
            {
                s_mem_area* mem_area             = memoryGetAreaTable();
                unsigned int mem_area_addr_start = mem_area->address;
                unsigned int mem_area_addr_end   = mem_area->address + mem_area->size;
                unsigned int mem_area_offset     = 0;

                // on RPLs we need to find the free area we can store data to (at least RPX was already loaded by this point)
                if(entryIndex > 0)
                    mem_area = rpxRplTableGetNextFreeMemArea(&mem_area_addr_start, &mem_area_addr_end, &mem_area_offset);

                rpl_struct = rpxRplTableAddEntry(filename, mem_area_offset, 0, fileType == 0, entryIndex, mem_area);
            }

            rpl_struct->size += rpxRplCopyDataToMem(rpl_struct, rpl_struct->size, (unsigned char*)load_address, bytes_loaded);
        }
    }

    // end of our little intrusion into this function
    //------------------------------------------------------------------------------------------------------------------

    // set the result to the global bounce error variable
    *sgBounceError = result;

    // disable global flag that buffer is still loaded by IOSU
	if(OS_FIRMWARE == 550)
    {
        unsigned int zeroBitCount = 0;
        asm volatile("cntlzw %0, %0" : "=r" (zeroBitCount) : "r"(*sgFinishedLoadingBuffer));
        *sgFinishedLoadingBuffer = zeroBitCount >> 5;
    }
    else
    {
        *sgIsLoadingBuffer = 0;
    }

    // check result for errors
    if(result == 0) {
        // the remaining size is set globally and in stack variable only
        // if a pointer was passed to this function
        if(iRemainingBytes) {
            *sgGotBytes = remaining_bytes;
            *iRemainingBytes = remaining_bytes;
            // on 5.5.x a new variable for total loaded bytes was added
            if(sgTotalBytes != NULL) {
                *sgTotalBytes += remaining_bytes;
            }
        }
        // Comment (Dimok):
        // calculate time difference and print it on logging how long the wait for asynchronous data load took
        // something like (systemTime2 - systemTime1) * constant / bus speed, did not look deeper into it as we don't need that crap
    }
    else {
        // Comment (Dimok):
        // a lot of error handling here. depending on error code sometimes calls Loader_Panic() -> we don't make errors so we can skip that part ;-P
    }
    return result;
}
/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/
#define MAKE_MAGIC(x, lib) { (unsigned int) my_ ## x, (unsigned int) &real_ ## x, lib, # x }

static const struct hooks_magic_t {
    const unsigned int replaceAddr;
    const unsigned int replaceCall;
    const unsigned int library;
    const char functionName[30];
} method_hooks[] = {
    MAKE_MAGIC(FSBindMount,                 LIB_CORE_INIT),
    // LOADER function
    MAKE_MAGIC(LiWaitOneChunk,              LIB_LOADER),

    // Dynamic RPL loading functions
    MAKE_MAGIC(OSDynLoad_Acquire,           LIB_CORE_INIT),
};

//! buffer to store our 2 instructions needed for our replacements
//! the code will be placed in the address of that buffer - CODE_RW_BASE_OFFSET
//! avoid this buffer to be placed in BSS and reset on start up
volatile unsigned int fs_method_calls[sizeof(method_hooks) / sizeof(struct hooks_magic_t) * 2] __attribute__((section(".data")));

void PatchMethodHooks(void)
{
    restore_instructions_t * restore = (restore_instructions_t *)(RESTORE_INSTR_ADDR);
    //! check if it is already patched
    if(restore->magic == RESTORE_INSTR_MAGIC)
        return;

    restore->magic = RESTORE_INSTR_MAGIC;
    restore->instr_count = 0;

    bat_table_t table;
    KernelSetDBATs(&table);

    /* Patch branches to it. */
    volatile unsigned int *space = &fs_method_calls[0];

    int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);

    for(int i = 0; i < method_hooks_count; i++)
    {
        unsigned int repl_addr = (unsigned int)method_hooks[i].replaceAddr;
        unsigned int call_addr = (unsigned int)method_hooks[i].replaceCall;

        unsigned int real_addr = 0;

        if(strcmp(method_hooks[i].functionName, "OSDynLoad_Acquire") == 0)
        {
            memcpy(&real_addr, &OSDynLoad_Acquire, 4);
        }
        else if(strcmp(method_hooks[i].functionName, "LiWaitOneChunk") == 0)
        {
            memcpy(&real_addr, &addr_LiWaitOneChunk, 4);
        }
        else
        {
            OSDynLoad_FindExport(coreinit_handle, 0, method_hooks[i].functionName, &real_addr);
        }

        // fill the restore instruction section
        restore->data[restore->instr_count].addr = real_addr;
        restore->data[restore->instr_count].instr = *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + real_addr);
        restore->instr_count++;

        // set pointer to the real function
        *(volatile unsigned int *)(call_addr) = (unsigned int)(space) - CODE_RW_BASE_OFFSET;
        DCFlushRange((void*)(call_addr), 4);

        // fill the instruction of the real function
        *space = *(volatile unsigned int*)(LIB_CODE_RW_BASE_OFFSET + real_addr);
        space++;

        // jump to real function skipping the first/replaced instruction
        *space = 0x48000002 | ((real_addr + 4) & 0x03fffffc);
        space++;
        DCFlushRange((void*)(space - 2), 8);
        ICInvalidateRange((unsigned char*)(space - 2) - CODE_RW_BASE_OFFSET, 8);

        unsigned int replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
        *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + real_addr) = replace_instr;
        DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + real_addr), 4);
        ICInvalidateRange((void*)(real_addr), 4);
    }

    KernelRestoreDBATs(&table);

    gLoaderPhysicalBufferAddr = (u32)OSEffectiveToPhysical((void*)0xF6000000);
    if(gLoaderPhysicalBufferAddr == 0)
        gLoaderPhysicalBufferAddr = 0x1B000000; // this is just in case and probably never needed
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInstructions(void)
{
    bat_table_t table;
    KernelSetDBATs(&table);

    restore_instructions_t * restore = (restore_instructions_t *)(RESTORE_INSTR_ADDR);
    if(restore->magic == RESTORE_INSTR_MAGIC)
    {
        for(unsigned int i = 0; i < restore->instr_count; i++)
        {
            *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + restore->data[i].addr) = restore->data[i].instr;
            DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + restore->data[i].addr), 4);
            ICInvalidateRange((void*)restore->data[i].addr, 4);
        }

    }
    restore->magic = 0;
    restore->instr_count = 0;

    KernelRestoreDBATs(&table);
    KernelRestoreInstructions();
}
