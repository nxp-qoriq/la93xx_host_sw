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

#define LL_TABLE_ADDR		0x58000000
#define LL_MAX_SIZE		0x10000
#define DEFAULT_TXRX_SIZE	0x20

typedef struct host_desc {
	uint32_t addr;
	uint32_t size;
} host_desc_t;

typedef struct mdm_desc {
	uint32_t addr;
	uint32_t size;
} mdm_desc_t;

int pci_dma_mem_init(uint32_t edma_base_addr, uint32_t edma_reg_size, uint32_t ll_base_addr, uint32_t nb_desc, int sg_en);
void pci_dma_mem_deinit(uint32_t edma_reg_size, uint32_t nb_desc);
void pci_dma_tx_reset(void);
void pci_dma_rx_reset(void);
int pci_dma_completed(int ch_num, int dir);
void pci_dma_read(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ch_num, int sg_en);
void pci_dma_write(uint32_t ddr_src, uint32_t pci_dst, uint32_t size, uint32_t ch_num, int sg_en);
void pci_dma_sg_configure(host_desc_t *host_desc, mdm_desc_t *mdm_desc, uint32_t nb_desc, int dir);
#endif /* __IMX_EDMA_API_H__ */
