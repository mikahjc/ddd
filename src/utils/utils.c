

void FlushRange(unsigned int startAddr, unsigned int size)
{
    register unsigned int addr = startAddr & ~0x1F;
    register unsigned int len = ((startAddr & 0x1F) + size) >> 5;

    while(len)
    {
        asm volatile("dcbf 0, %0" : : "r"(addr));
        addr += 0x20;
        --len;
    }
    asm volatile("sync; eieio");
}


void InvalidateRange(unsigned int startAddr, unsigned int size)
{
    register unsigned int addr = startAddr & ~0x1F;
    register unsigned int len = ((startAddr & 0x1F) + size) >> 5;

    while(len)
    {
        asm volatile("dcbi 0, %0" : : "r"(addr));
        addr += 0x20;
        --len;
    }
}
