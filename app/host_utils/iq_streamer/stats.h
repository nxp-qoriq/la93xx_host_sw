// =============================================================================
//! @file       main.h
//! @brief      main interface.
//! @author     NXP Semiconductors.
//! @copyright  Copyright 2023-2024 NXP
// =============================================================================
/*
*  NXP Confidential. This software is owned or controlled by NXP and may only be used strictly
*  in accordance with the applicable license terms. By expressly accepting
*  such terms or by downloading, installing, activating and/or otherwise using
*  the software, you are agreeing that you have read, and that you agree to
*  comply with and are bound by, such license terms. If you do not agree to
*  be bound by the applicable license terms, then you may not retain,
*  install, activate or otherwise use the software.
*/

#ifndef __STATS_H__
#define __STATS_H__

#ifdef __VSPA__
#include "iqmod_rx.h"
#endif

typedef enum {
	STAT_DMA_AXIQ_READ,             // 0x1
	STAT_DMA_DDR_WR,                // 0x3
	STAT_EXT_DMA_DDR_WR,            // 0x5
	ERROR_DMA_DDR_WR_OVERRUN,       // 0x7
	ERROR_AXIQ_FIFO_RX_UNDERRUN,    // 0xa
	ERROR_AXIQ_FIFO_RX_OVERRUN,     // 0xb
	ERROR_AXIQ_DMA_RX_CMD_OVERRUN,  // 0xd
	ERROR_EXT_DMA_DDR_WR_OVERRUN,   // 0x11
	STATS_RX_MAX
} stats_rx_e;

typedef enum {
	STAT_DMA_AXIQ_WRITE = 0,          // 0x0
	STAT_DMA_DDR_RD,                // 0x2
	STAT_EXT_DMA_DDR_RD,            // 0x4
	ERROR_DMA_DDR_RD_UNDERRUN,      // 0x6
	ERROR_AXIQ_FIFO_TX_UNDERRUN,    // 0x8
	ERROR_AXIQ_FIFO_TX_OVERRUN,     // 0x9
	ERROR_AXIQ_DMA_TX_CMD_UNDERRUN, // 0xc
	ERROR_EXT_DMA_DDR_RD_UNDERRUN,   // 0x11
	STATS_TX_MAX
} stats_tx_e;

typedef enum {
	ERROR_DMA_CONFIG_ERROR,         // 0xe
	ERROR_DMA_XFER_ERROR,           // 0xf
	STATS_GBL_MAX
} stats_gbl_e;

typedef struct s_stats {
	uint32_t gbl_stats[STATS_GBL_MAX];
	uint32_t tx_stats[STATS_TX_MAX];
	uint32_t rx_stats[RX_NUM_CHAN][STATS_RX_MAX];
} t_stats;


#ifndef __VSPA__

static char *VSPA_stat_rx_string[STATS_RX_MAX+1] = {
	"DMA_AXIQ_RD",
	"DMA_DDR_WR", 
	"EXT_DDR_WR", 
	"DDR_WR_OVR", 
	"FIFO_RX_UDR",
	"FIFO_RX_OVR",
	"DMA_RX_CMD_OVR",
	"EXT_DDR_WR_OVR",
	"STATS_RX_MAX"
};

static char *VSPA_stat_tx_string[STATS_TX_MAX+1] = {
	"DMA_AXIQ_WR",
	"DMA_DDR_RD", 
	"EXT_DDR_RD", 
	"DDR_RD_UDR", 
	"FIFO_TX_UDR",
	"FIFO_TX_OVR",
	"DMA_TX_CMD_UDR",
	"EXT_DDR_RD_UDR",
	"STATS_TX_MAX"
};

static char *VSPA_stat_gbl_string[STATS_GBL_MAX+1] = {
	"DMA_CFG_ERROR",
	"DMA_XFER_ERROR", 
	"STATS_GBL_MAX"
};

#endif

#ifdef __VSPA__
  extern volatile t_stats g_stats;
#endif


#endif // __STATS_H__
