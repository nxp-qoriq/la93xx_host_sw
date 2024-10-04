/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2022-2024 NXP
*/


#ifndef __STATS_H__
#define __STATS_H__

#define pr_info printf
#define MAX_CYC_CNT 20

typedef enum {
	STAT_DMA_AXIQ_WRITE = 0,          // 0x0
	STAT_DMA_AXIQ_READ,             // 0x1
	STAT_DMA_DDR_RD,                // 0x2
	STAT_DMA_DDR_WR,                // 0x3
	STAT_EXT_DMA_DDR_RD,            // 0x4
	STAT_EXT_DMA_DDR_WR,            // 0x5
	ERROR_DMA_DDR_RD_UNDERRUN,      // 0x6
	ERROR_DMA_DDR_WR_OVERRUN,       // 0x7
	ERROR_AXIQ_FIFO_TX_UNDERRUN,    // 0x8
	ERROR_AXIQ_FIFO_TX_OVERRUN,     // 0x9
	ERROR_AXIQ_FIFO_RX_UNDERRUN,    // 0xa
	ERROR_AXIQ_FIFO_RX_OVERRUN,     // 0xb
	ERROR_AXIQ_DMA_TX_CMD_UNDERRUN, // 0xc
	ERROR_AXIQ_DMA_RX_CMD_OVERRUN,  // 0xd
	ERROR_DMA_CONFIG_ERROR,         // 0xe
	ERROR_DMA_XFER_ERROR,           // 0xf
	ERROR_EXT_DMA_DDR_RD_UNDERRUN,  // 0x10
	ERROR_EXT_DMA_DDR_WR_OVERRUN,   // 0x11
	STATS_MAX
} stats_e;

#define GET_VSPA_CYCLE_COUNT(table, ant, index) {\
	if (!stats_locked) {\
	table[ant][index] = (uint64_t)((*_VSPA_cyc_count_msb)&0x7FFFFFFF)*0x100000000;\
	table[ant][index] +=  *_VSPA_cyc_count_lsb;\
	} \
	}

void la9310_hexdump(const void *ptr, size_t sz);
void print_history(void);
void print_vspa_stats(void);
void monitor_vspa_stats(void);
void print_vspa_trace(void);


#endif
