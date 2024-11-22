/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __IQ_STREAMER_H__
#define __IQ_STREAMER_H__

#include "imx8-host.h"

typedef enum {
	HOST_DMA_OFF = 0,
	HOST_DMA_LINUX,
	HOST_DMA_M7,
	HOST_DMA_MAX
} host_dma_e;

#define NB_RCHAN 1
#define NB_WCHAN 1

#ifdef IQMOD_RX_1R
#define EXT_DMA_TX_DDR_STEP (2*TX_DDR_STEP)
#define EXT_DMA_RX_DDR_STEP (2*RX_DDR_STEP)
#else
#define EXT_DMA_TX_DDR_STEP (1*TX_DDR_STEP)
#define EXT_DMA_RX_DDR_STEP (1*RX_DDR_STEP)
#endif

extern uint32_t *BAR0_addr;
extern uint32_t *BAR1_addr;
extern uint32_t *BAR2_addr;
extern uint32_t *PCIE1_addr;
extern t_stats *host_stats;
extern uint32_t *v_tx_vspa_proxy_wo;
extern uint32_t *v_rx_vspa_proxy_wo;


#ifdef IQMOD_RX_1R
extern uint32_t local_RX_total_produced_size;
extern uint32_t local_RX_total_consumed_size;
extern uint32_t local_DDR_wr_load_start_bit_update;
#else
typedef struct s_local_rx_ch_context {
	uint32_t RX_total_produced_size;
	uint32_t RX_total_consumed_size;
	uint32_t ddr_wr_offset;
	uint32_t DDR_wr_base_address;
	uint32_t DDR_wr_size;
	uint32_t dmem_src_offset;
	uint32_t dmem_rd_base_address;
} t_local_rx_ch_context;
extern t_local_rx_ch_context local_rx_ch_context[];
extern uint32_t local_DDR_wr_load_start_bit_update;
#endif

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
extern uint32_t *v_vspa_dmem_proxy;
extern uint32_t *v_scratch_ddr_addr;
extern volatile uint32_t *_VSPA_cyc_count_lsb;
extern volatile uint32_t *_VSPA_cyc_count_msb;

#endif
