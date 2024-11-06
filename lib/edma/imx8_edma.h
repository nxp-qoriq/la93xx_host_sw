/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __IMX_EDMA_H__
#define __IMX_EDMA_H__

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

#define DMA_EN				0x1
#define DMA_PW_EN			0x1
#define DMA_CONT_VAL			0x04000008

#endif /* __IMX_EDMA_API_H__ */
