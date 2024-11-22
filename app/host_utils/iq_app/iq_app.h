/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __IQ_APP_H__
#define __IQ_APP_H__

typedef enum {
	OP_TX_ONLY = 0,
	OP_RX_ONLY,
	OP_STATS,
	OP_DUMP_TRACE,
	OP_DMA_PERF,
	OP_CLEAR_STATS,
	OP_MAX
} command_e;


extern volatile uint32_t running;
extern uint32_t *v_iqflood_ddr_addr;
extern uint32_t *v_scratch_ddr_addr;

#endif
