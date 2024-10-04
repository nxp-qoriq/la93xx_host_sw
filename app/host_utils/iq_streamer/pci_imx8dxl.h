/* Copyright 2022-2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PCI_IMX8MP_H__
#define __PCI_IMX8MP_H__

int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size);
int PCI_DMA_WRITE_transfer(uint32_t ddr_src, uint32_t pci_dst, uint32_t size);
void dma_perf_test(void);
extern uint32_t *PCIE1_addr;


#define IMX8MP_PCIE1_ADDR  0x5F010000
#define IMX8MP_PCIE1_SIZE  0x10000

#define PCIE_DMA_BASE (uint64_t)PCIE1_addr
#define DMA_READ_ENGINE_EN_OFF          (PCIE_DMA_BASE + 0x99C)
#define DMA_READ_INT_MASK_OFF           (PCIE_DMA_BASE + 0xA18)
#define DMA_CH_CONTROL1_OFF_RDCH_0      (PCIE_DMA_BASE + 0xA70)
#define DMA_TRANSFER_SIZE_OFF_RDCH_0    (PCIE_DMA_BASE + 0xA78)
#define DMA_SAR_LOW_OFF_RDCH_0          (PCIE_DMA_BASE + 0xA7C)
#define DMA_SAR_HIGH_OFF_RDCH_0         (PCIE_DMA_BASE + 0xA80)
#define DMA_DAR_LOW_OFF_RDCH_0          (PCIE_DMA_BASE + 0xA84)
#define DMA_DAR_HIGH_OFF_RDCH_0         (PCIE_DMA_BASE + 0xA88)
#define DMA_READ_DOORBELL_OFF           (PCIE_DMA_BASE + 0x9A0)
#define DMA_READ_INT_CLEAR_OFF          (PCIE_DMA_BASE + 0xA1C)
#define DMA_WRITE_ENGINE_EN_OFF         (PCIE_DMA_BASE + 0x97C)
#define DMA_WRITE_INT_MASK_OFF          (PCIE_DMA_BASE + 0x9C4)
#define DMA_CH_CONTROL1_OFF_WRCH_0      (PCIE_DMA_BASE + 0xA70)
#define DMA_TRANSFER_SIZE_OFF_WRCH_0    (PCIE_DMA_BASE + 0xA78)
#define DMA_SAR_LOW_OFF_WRCH_0          (PCIE_DMA_BASE + 0xA7C)
#define DMA_SAR_HIGH_OFF_WRCH_0         (PCIE_DMA_BASE + 0xA80)
#define DMA_DAR_LOW_OFF_WRCH_0          (PCIE_DMA_BASE + 0xA84)
#define DMA_DAR_HIGH_OFF_WRCH_0         (PCIE_DMA_BASE + 0xA88)
#define DMA_WRITE_DOORBELL_OFF          (PCIE_DMA_BASE + 0x980)
#define DMA_WRITE_INT_CLEAR_OFF         (PCIE_DMA_BASE + 0x9C8)
#define DMA_WRITE_INT_STATUS_OFF		(PCIE_DMA_BASE + 0x9BC)

#define DMA_WRITE_LINKED_LIST_ERR_EN_OFF (PCIE_DMA_BASE + 0xAA0)
#define DMA_LLP_LOW_OFF_RDCH_0			 (PCIE_DMA_BASE + 0xA8c)
#define DMA_LLP_HIGH_OFF_RDCH_0			 (PCIE_DMA_BASE + 0xA90)

#endif
