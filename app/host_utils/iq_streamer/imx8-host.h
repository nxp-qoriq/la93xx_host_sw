/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __IMX8_HOST_H__
#define __IMX8_HOST_H__

typedef enum {
	HOST_DMA_OFF = 0,
	HOST_DMA_LINUX,    
	HOST_DMA_M7,
	HOST_DMA_MAX
} host_dma_e;

#define NB_RCHAN 2
#define NB_WCHAN 2
#define EXT_DMA_TX_DDR_STEP (2*TX_DDR_STEP)
#define EXT_DMA_RX_DDR_STEP (2*RX_DDR_STEP)

extern uint32_t *BAR0_addr;
extern uint32_t *BAR1_addr;
extern uint32_t *BAR2_addr;
extern uint32_t *PCIE1_addr;

// Data Cache Flush/Clean
#define dcbf(p) { asm volatile("dc cvac, %0" : : "r"(p) : "memory"); }
// Clean and Invalidate data cache
#define dccivac(p) { asm volatile("dc civac, %0" : : "r"(p) : "memory"); }

#ifndef IMX8DXL
#define IQFLOOD_VSPA_PROXY_SIZE 0x100

#define BAR0_SIZE	0x4000000
#define BAR0_ADDR	0x18000000
#define BAR1_SIZE	0x20000
#define BAR1_ADDR	0x1C000000
#define BAR2_SIZE	0x800000
#define BAR2_ADDR	0x1f000000
#define IMX8MP_PCIE1_ADDR   0x33800000
#define IMX8MP_PCIE1_SIZE  0x400000

#define OCRAM_SIZE	0x4000000
#define OCRAM_ADDR     0x900000

#else

#define BAR0_SIZE       0x4000000
#define BAR0_ADDR       0x70000000
#define BAR1_SIZE       0x20000
#define BAR1_ADDR       0x75000000
#define BAR2_SIZE       0x800000
#define BAR2_ADDR       0x78000000
#endif

#endif
