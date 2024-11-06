/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __IMX_EDMA_API_H__
#define __IMX_EDMA_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>

#define IMX8MP_DMA_BASE_ADDR	0x33b80000
#define IMX8MP_DMA_REG_SPACE_SIZE	0x80000

/* Need to be implemented for DXL */
#define IMX8DXL_DMA_BASE_ADDRESS  0x5F010000
#define IMX8DXL_DMA_REG_SPACE_SIZE  0x10000

int init_mem(uint32_t edma_base_addr, uint32_t edma_reg_size);
void deinit_mem(uint32_t edma_reg_size);
void pci_dma_tx_reset(void);
void pci_dma_rx_reset(void);
int pci_dma_completed(int ch_num, int dir);
void pci_dma_read(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ch_num);
void pci_dma_write(uint32_t ddr_src, uint32_t pci_dst, uint32_t size, uint32_t ch_num);
#endif /* __IMX_EDMA_API_H__ */
