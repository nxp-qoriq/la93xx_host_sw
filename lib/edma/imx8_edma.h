/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __IMX_EDMA_H__
#define __IMX_EDMA_H__

#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>

#define MAP_MASK (MAP_SIZE - 1)
#define DMA_WRITE_ENABLE_REG		0xc
#define DMA_WRITE_PW_ENABLE_BASE_REG	0x124
#define DMA_WRITE_INT_MASK_REG		0x54
#define DMA_WRITE_DOORBELL_REG		0x10
#define DMA_READ_ENABLE_REG		0x2c
#define DMA_READ_PW_ENABLE_BASE_REG	0x164
#define DMA_READ_INT_MASK_REG		0xa8
#define DMA_READ_DOORBELL_REG		0x30
#define DMA_CHANNEL_CTXT_OFF_REG	0x200
#define DMA_CHANNEL_READ_CTXT_OFF_REG	0x100
#define DMA_CHANNEL_CTRL_REG		0x0
#define DMA_SIZE_REG			0x8
#define DMA_SRC_LOW_REG			0xc
#define DMA_SRC_HIGH_REG		0x10
#define DMA_DST_LOW_REG			0x14
#define DMA_DST_HIGH_REG		0x18
#define DMA_LL_LOW_REG			0x1c
#define DMA_LL_HIGH_REG			0x20
#define DMA_WRITE_LL_ERR_EN		0x90
#define DMA_READ_LL_ERR_EN		0xC4
#define LL_TOTAL_SIZE			0x8000
#define LL_BUF_SIZE			0x10
#define SRC_LL_NUM			16

#define lower_32_bits(n) ((uint32_t)(n))
#define upper_32_bits(n) ((uint32_t)(((n) >> 16) >> 16))

#define DMA_EN				0x1
#define DMA_PW_EN			0x1
#define DMA_CONT_VAL			0x04000008
#define DMA_LL_CONT_VAL		0x04000300

typedef struct dw_edma_v0_lli {
	uint32_t control;
	uint32_t transfer_size;
	union {
		uint64_t reg;
		struct {
			uint32_t lsb;
			uint32_t msb;
		};
	} sar;
	union {
		uint64_t reg;
		struct {
			uint32_t lsb;
			uint32_t msb;
		};
	} dar;
} edma_ll_table;

#endif /* __IMX_EDMA_API_H__ */
