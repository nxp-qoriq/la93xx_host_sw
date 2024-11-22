/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __IMX8_HOST_H__
#define __IMX8_HOST_H__



#define CACHE_LINE_SIZE 64

// Data Cache Flush/Clean
#define dcbf(p) { asm volatile("dc cvac, %0" : : "r"(p) : "memory"); }
// Clean and Invalidate data cache
#define dccivac(p) { asm volatile("dc civac, %0" : : "r"(p) : "memory"); }

void invalidate_region(void* region, uint32_t size);
void flush_region(void *region, uint32_t size);


#ifndef IMX8DXL
#define PCIE1_ADDR   0x33800000
#define PCIE1_SIZE  0x400000
#define DMA_OFFSET   0x380000

#define BAR0_SIZE	0x4000000
#define BAR0_ADDR	0x18000000
#define BAR1_SIZE	0x20000
#define BAR1_ADDR	0x1C000000
#define BAR2_SIZE	0x800000
#define BAR2_ADDR	0x1f000000

#define OCRAM_SIZE	0x4000000
#define OCRAM_ADDR     0x900000

#else
#define PCIE1_ADDR   0x5F010000
#define PCIE1_SIZE  0x10000
#define DMA_OFFSET   0x970

#define BAR0_SIZE       0x4000000
#define BAR0_ADDR       0x70000000
#define BAR1_SIZE       0x20000
#define BAR1_ADDR       0x75000000
#define BAR2_SIZE       0x800000
#define BAR2_ADDR       0x78000000
#endif

#endif
