/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __IQ_STREAMER_H__
#define __IQ_STREAMER_H__

#include "imx8-host.h"

#define TCM_START   0x20000000
#define TCM_TX_FIFO 0x20001000

typedef enum {
	OP_TX_ONLY = 0,
	OP_RX_ONLY,    
	OP_TX_RX,      
	OP_STATS,      
	OP_MONITOR,    
	OP_DUMP_TRACE, 
	OP_DMA_PERF, 
	OP_DUMP_TRACE_M7,
	OP_CLEAR_STATS,
	OP_MAX
} command_e;


extern volatile uint32_t running;
extern uint32_t *v_iqflood_ddr_addr;
extern uint32_t *v_ocram_addr;
extern uint32_t *v_scratch_ddr_addr;
extern volatile uint32_t *_VSPA_cyc_count_lsb;
extern volatile uint32_t *_VSPA_cyc_count_msb;

#endif
