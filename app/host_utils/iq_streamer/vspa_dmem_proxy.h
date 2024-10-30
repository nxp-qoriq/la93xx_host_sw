/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef VSPA_DMEM_PROXY_H_
#define VSPA_DMEM_PROXY_H_
#include "iqmod_rx.h"
#include "stats.h"

// IPC region in iqflood

#define VSPA_DMEM_PROXY_SIZE  256

typedef struct s_tx_readonly {
	uint32_t TX_total_consumed_size; // should be first for alignment reason
	uint32_t cmd_start;
	uint32_t DDR_rd_base_address;
	uint32_t DDR_rd_size;
} t_tx_readonly;

typedef struct s_vspa_dmem_proxy {
//	t_tx_readonly  tx_state_readonly;
//	t_stats vspa_stats;
	t_stats host_stats;
} t_vspa_dmem_proxy;

extern t_vspa_dmem_proxy *v_vspa_dmem_proxy;

#endif /* IQMOD_TX_H_ */
