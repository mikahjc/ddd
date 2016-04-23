/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <malloc.h>
#include <string.h>
#include <gctypes.h>
#include "common/common.h"
#include "memory_area_table.h"

typedef struct _memory_values_t
{
    unsigned int start_address;
    unsigned int end_address;
} memory_values_t;

static const memory_values_t mem_vals_500[] =
{
    { 0x2E605CBC, 0x2FF849BC }, // size 26733828 (26107 kB) (25 MB)
    { 0x2CAE7878, 0x2D207DB4 }, // size 7472448 (7297 kB) (7 MB)
    { 0x2D3B966C, 0x2D8943C0 }, // size 5090648 (4971 kB) (4 MB)
    { 0x2D8AD3D8, 0x2DFFFFFC }, // size 7679016 (7499 kB) (7 MB)
    // TODO: Check which of those areas are usable
//		{ 0x283A73DC, 0x284D2AE4 } // size 1226508 (1197 kB) (1 MB)
//		{ 0x29030800, 0x293F69FC } // size 3957248 (3864 kB) (3 MB)
//		{ 0x2970200C, 0x298B9C54 } // size 1801292 (1759 kB) (1 MB)
//		{ 0x2A057B68, 0x2A1B9578 } // size 1448468 (1414 kB) (1 MB)

//		{ 0x29030800, 0x293F69FC } // size 3957248 (3864 kB) (3 MB)
//		{ 0x2970200C, 0x298B9C54 } // size 1801292 (1759 kB) (1 MB)
//		{ 0x2A057B68, 0x2A1B9578 } // size 1448468 (1414 kB) (1 MB)
//		{ 0x288EEC30, 0x28B06E94 } // size 2196072 (2144 kB) (2 MB)
//		{ 0x283A73DC, 0x284D2AE4 } // size 1226508 (1197 kB) (1 MB)
//		{ 0x3335A4C0, 0x335021FC } // size 1736000 (1695 kB) (1 MB)
//		{ 0x3350C1D4, 0x339182CC } // size 4243708 (4144 kB) (4 MB)
//		{ 0x33A14094, 0x33EBFFFC } // size 4898668 (4783 kB) (4 MB)
//		{ 0x33FFFFD4, 0x38FFFFFC } // size 83886124 (81920 kB) (80 MB)
    {0, 0}
};

static const memory_values_t mem_vals_532[] =
{
    // TODO: Check which of those areas are usable
//        {0x28000000 + 0x000DCC9C, 0x28000000 + 0x00174F80}, // 608 kB
//        {0x28000000 + 0x00180B60, 0x28000000 + 0x001C0A00}, // 255 kB
//        {0x28000000 + 0x001ECE9C, 0x28000000 + 0x00208CC0}, // 111 kB
//        {0x28000000 + 0x00234180, 0x28000000 + 0x0024B444}, // 92 kB
//        {0x28000000 + 0x0024D8C0, 0x28000000 + 0x0028D884}, // 255 kB
//        {0x28000000 + 0x003A745C, 0x28000000 + 0x004D2B68}, // 1197 kB
//        {0x28000000 + 0x004D77B0, 0x28000000 + 0x00502200}, // 170 kB
//        {0x28000000 + 0x005B3A88, 0x28000000 + 0x005C6870}, // 75 kB
//        {0x28000000 + 0x0061F3E4, 0x28000000 + 0x00632B04}, // 77 kB
//        {0x28000000 + 0x00639790, 0x28000000 + 0x00649BC4}, // 65 kB
//        {0x28000000 + 0x00691490, 0x28000000 + 0x006B3CA4}, // 138 kB
//        {0x28000000 + 0x006D7BCC, 0x28000000 + 0x006EEB84}, // 91 kB
//        {0x28000000 + 0x00704E44, 0x28000000 + 0x0071E3C4}, // 101 kB
//        {0x28000000 + 0x0073B684, 0x28000000 + 0x0074C184}, // 66 kB
//        {0x28000000 + 0x00751354, 0x28000000 + 0x00769784}, // 97 kB
//        {0x28000000 + 0x008627DC, 0x28000000 + 0x00872904}, // 64 kB
//        {0x28000000 + 0x008C1E98, 0x28000000 + 0x008EB0A0}, // 164 kB
//        {0x28000000 + 0x008EEC30, 0x28000000 + 0x00B06E98}, // 2144 kB
//        {0x28000000 + 0x00B06EC4, 0x28000000 + 0x00B930C4}, // 560 kB
//        {0x28000000 + 0x00BA1868, 0x28000000 + 0x00BC22A4}, // 130 kB
//        {0x28000000 + 0x00BC48F8, 0x28000000 + 0x00BDEC84}, // 104 kB
//        {0x28000000 + 0x00BE3DC0, 0x28000000 + 0x00C02284}, // 121 kB
//        {0x28000000 + 0x00C02FC8, 0x28000000 + 0x00C19924}, // 90 kB
//        {0x28000000 + 0x00C2D35C, 0x28000000 + 0x00C3DDC4}, // 66 kB
//        {0x28000000 + 0x00C48654, 0x28000000 + 0x00C6E2E4}, // 151 kB
//        {0x28000000 + 0x00D04E04, 0x28000000 + 0x00D36938}, // 198 kB
//        {0x28000000 + 0x00DC88AC, 0x28000000 + 0x00E14288}, // 302 kB
//        {0x28000000 + 0x00E21ED4, 0x28000000 + 0x00EC8298}, // 664 kB
//        {0x28000000 + 0x00EDDC7C, 0x28000000 + 0x00F7C2A8}, // 633 kB
//        {0x28000000 + 0x00F89EF4, 0x28000000 + 0x010302B8}, // 664 kB
//        {0x28000000 + 0x01030800, 0x28000000 + 0x013F69A0}, // 3864 kB
//        {0x28000000 + 0x016CE000, 0x28000000 + 0x016E0AA0}, // 74 kB
//        {0x28000000 + 0x0170200C, 0x28000000 + 0x018B9C58}, // 1759 kB
//        {0x28000000 + 0x01F17658, 0x28000000 + 0x01F6765C}, // 320 kB
//        {0x28000000 + 0x01F6779C, 0x28000000 + 0x01FB77A0}, // 320 kB
//        {0x28000000 + 0x01FB78E0, 0x28000000 + 0x020078E4}, // 320 kB
//        {0x28000000 + 0x02007A24, 0x28000000 + 0x02057A28}, // 320 kB
//        {0x28000000 + 0x02057B68, 0x28000000 + 0x021B957C}, // 1414 kB
//        {0x28000000 + 0x02891528, 0x28000000 + 0x028C8A28}, // 221 kB
//        {0x28000000 + 0x02BBCC4C, 0x28000000 + 0x02CB958C}, // 1010 kB
//        {0x28000000 + 0x0378D45C, 0x28000000 + 0x03855464}, // 800 kB
//        {0x28000000 + 0x0387800C, 0x28000000 + 0x03944938}, // 818 kB
//        {0x28000000 + 0x03944A08, 0x28000000 + 0x03956E0C}, // 73 kB
//        {0x28000000 + 0x04A944A4, 0x28000000 + 0x04ABAAC0}, // 153 kB
//        {0x28000000 + 0x04ADE370, 0x28000000 + 0x0520EAB8}, // 7361 kB      // ok
//        {0x28000000 + 0x053B966C, 0x28000000 + 0x058943C4}, // 4971 kB      // ok
//        {0x28000000 + 0x058AD3D8, 0x28000000 + 0x06000000}, // 7499 kB
//        {0x28000000 + 0x0638D320, 0x28000000 + 0x063B0280}, // 139 kB
//        {0x28000000 + 0x063C39E0, 0x28000000 + 0x063E62C0}, // 138 kB
//        {0x28000000 + 0x063F52A0, 0x28000000 + 0x06414A80}, // 125 kB
//        {0x28000000 + 0x06422810, 0x28000000 + 0x0644B2C0}, // 162 kB
//        {0x28000000 + 0x064E48D0, 0x28000000 + 0x06503EC0}, // 125 kB
//        {0x28000000 + 0x0650E360, 0x28000000 + 0x06537080}, // 163 kB
//        {0x28000000 + 0x0653A460, 0x28000000 + 0x0655C300}, // 135 kB
//        {0x28000000 + 0x0658AA40, 0x28000000 + 0x065BC4C0}, // 198 kB       // ok
//        {0x28000000 + 0x065E51A0, 0x28000000 + 0x06608E80}, // 143 kB       // ok
//        {0x28000000 + 0x06609ABC, 0x28000000 + 0x07F82C00}, // 26084 kB     // ok

//        {0x30000000 + 0x000DCC9C, 0x30000000 + 0x00180A00}, // 655 kB
//        {0x30000000 + 0x00180B60, 0x30000000 + 0x001C0A00}, // 255 kB
//        {0x30000000 + 0x001F5EF0, 0x30000000 + 0x00208CC0}, // 75 kB
//        {0x30000000 + 0x00234180, 0x30000000 + 0x0024B444}, // 92 kB
//        {0x30000000 + 0x0024D8C0, 0x30000000 + 0x0028D884}, // 255 kB
//        {0x30000000 + 0x003A745C, 0x30000000 + 0x004D2B68}, // 1197 kB
//        {0x30000000 + 0x006D3334, 0x30000000 + 0x00772204}, // 635 kB
//        {0x30000000 + 0x00789C60, 0x30000000 + 0x007C6000}, // 240 kB
//        {0x30000000 + 0x00800000, 0x30000000 + 0x01E20000}, // 22876 kB     // ok
    { 0x2E609ABC, 0x2FF82C00 }, // 26084 kB
    { 0x29030800, 0x293F69A0 }, // 3864 kB
    { 0x288EEC30, 0x28B06E98 }, // 2144 kB
    { 0x2D3B966C, 0x2D8943C4 }, // 4971 kB
    { 0x2CAE0370, 0x2D20EAB8 }, // 7361 kB
    { 0x2D8AD3D8, 0x2E000000 }, // 7499 kB

    {0, 0}
}; // total : 66mB + 25mB

static const memory_values_t mem_vals_540[] =
{
    { 0x2E609EFC, 0x2FF82BC0 }, // 26083 kB
    { 0x2D8AD3D8, 0x2E000000 }, // 7499 kB
    { 0x2CB56370, 0x2D1EF6B8 }, // 6756 kB
    { 0x2D3B966C, 0x2D8943C4 }, // 4971 kB
    { 0x29030800, 0x293F6A04 }, // 3864 kB
    { 0x288EEC30, 0x28B06E98 }, // 2144 kB
    { 0x2970200C, 0x298B9C58 }, // 1759 kB
    { 0x28B06EC4, 0x28B930C4 }, // 560 kB

    {0, 0}
};

//! retain container for our memory area table data
static unsigned char ucMemAreaTableBuffer[0xff];

s_mem_area * memoryGetAreaTable(void)
{
    return (s_mem_area *) (ucMemAreaTableBuffer);
}

static inline void memoryAddArea(int start, int end, int cur_index)
{
    // Create and copy new memory area
    s_mem_area * mem_area = memoryGetAreaTable();
    mem_area[cur_index].address = start;
    mem_area[cur_index].size    = end - start;
    mem_area[cur_index].next    = 0;

    // Fill pointer to this area in the previous area
    if (cur_index > 0)
    {
        mem_area[cur_index - 1].next = &mem_area[cur_index];
    }
}

/* Create memory areas arrays */
void memoryInitAreaTable()
{
    u32 ApplicationMemoryEnd;

    asm volatile("lis %0, __CODE_END@h; ori %0, %0, __CODE_END@l" : "=r" (ApplicationMemoryEnd));

    // This one seems to be available on every firmware and therefore its our code area but also our main RPX area behind our code
    // 22876 kB - our application    // ok
    memoryAddArea(ApplicationMemoryEnd + 0x30000000, 0x31E20000, 0);

    const memory_values_t * mem_vals = NULL;

    switch(OS_FIRMWARE)
    {
    case 500: {
        mem_vals = mem_vals_500;
        break;
    }
    case 532: {
        mem_vals = mem_vals_532;
        break;
    }
    case 540:
	case 550: {
        mem_vals = mem_vals_540;
        break;
    }
    default:
        return; // no known values
    }

    // Fill entries
    int i = 0;
    while (mem_vals[i].start_address)
    {
        memoryAddArea(mem_vals[i].start_address, mem_vals[i].end_address, i + 1);
        i++;
    }
}
