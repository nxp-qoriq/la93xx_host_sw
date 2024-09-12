/* Copyright 2022-2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef __IQ_STREAMER_H__
#define __IQ_STREAMER_H__

typedef enum
{
	OP_TX_ONLY=0,  // 0x0
	OP_RX_ONLY,    // 0x1        
	OP_TX_RX,      // 0x2
	OP_STATS,      // 0x3
	OP_MONITOR,    // 0x4
	OP_DUMP_TRACE, // 0x5
	OP_MAX
} command_e;

#define NUM_ANT 2

#define MAX_CYC_CNT 20

#define SIZE_4K  		(4096)
#define TX_NUM_BUF 8
#define RX_NUM_BUF 5
#define DMA_TXR_size	(512)
#define DDR_STEP	 	(4 * DMA_TXR_size)

#define MODEM_BAR0_CCSR_OFFSET	0x18000000
#define MODEM_CCSR_SIZE		    0x4000000
#define IQFLOOD_BUF_SIZE	 0xd000000
#define IQFLOOD_BUF_ADDR	0x96400000
#define BAR0_SIZE	0x4000000
#define BAR0_ADDR	0x18000000
#define BAR1_SIZE	0x20000
#define BAR1_ADDR	0x1C000000
#define BAR2_SIZE	0x800000
#define BAR2_ADDR	0x1f000000
#define IMX8MP_PCIE1_ADDR   0x33800000
#define IMX8MP_PCIE1_SIZE  0x400000


extern volatile uint32_t running;

extern uint32_t *iqflood_ddr_addr;
extern uint32_t *BAR0_addr;
extern uint32_t *BAR1_addr;
extern uint32_t *BAR2_addr;
extern uint32_t *PCIE1_addr;

extern volatile uint32_t *_VSPA_cyc_count_lsb;
extern volatile uint32_t *_VSPA_cyc_count_msb;

#endif
