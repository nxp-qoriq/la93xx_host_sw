/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */


#ifndef __PCI_IMX8MP_H__
#define __PCI_IMX8MP_H__

int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size,uint32_t nbchan);
int PCI_DMA_WRITE_transfer(uint32_t ddr_src, uint32_t pci_dst, uint32_t size,uint32_t nbchan);
int PCI_DMA_WRITE_completion(uint32_t nbchan);
int PCI_DMA_READ_completion(uint32_t nbchan);

void dma_perf_test(void);
extern uint32_t *PCIE1_addr;

#define IMX8MP_PCIE1_ADDR   0x33800000
#define IMX8MP_PCIE1_SIZE  0x400000

#ifdef __M7__
#define PCIE_DMA_BASE (uint32_t)PCIE1_addr
#else
#define PCIE_DMA_BASE (uint64_t)PCIE1_addr
#endif
#define DMA_READ_ENGINE_EN_OFF          (PCIE_DMA_BASE + 0x38002C)
#define DMA_READ_INT_MASK_OFF           (PCIE_DMA_BASE + 0x3800A8)
#define DMA_CH_CONTROL1_OFF_RDCH_0      (PCIE_DMA_BASE + 0x380300)
#define DMA_TRANSFER_SIZE_OFF_RDCH_0    (PCIE_DMA_BASE + 0x380308)
#define DMA_SAR_LOW_OFF_RDCH_0          (PCIE_DMA_BASE + 0x38030C)
#define DMA_SAR_HIGH_OFF_RDCH_0         (PCIE_DMA_BASE + 0x380310)
#define DMA_DAR_LOW_OFF_RDCH_0          (PCIE_DMA_BASE + 0x380314)
#define DMA_DAR_HIGH_OFF_RDCH_0         (PCIE_DMA_BASE + 0x380318)
#define DMA_READ_DOORBELL_OFF           (PCIE_DMA_BASE + 0x380030)
#define DMA_READ_INT_CLEAR_OFF          (PCIE_DMA_BASE + 0x3800AC)
#define DMA_WRITE_ENGINE_EN_OFF         (PCIE_DMA_BASE + 0x38000C)
#define DMA_WRITE_INT_MASK_OFF          (PCIE_DMA_BASE + 0x380054)
#define DMA_CH_CONTROL1_OFF_WRCH_0      (PCIE_DMA_BASE + 0x380200)
#define DMA_TRANSFER_SIZE_OFF_WRCH_0    (PCIE_DMA_BASE + 0x380208)
#define DMA_SAR_LOW_OFF_WRCH_0          (PCIE_DMA_BASE + 0x38020C)
#define DMA_SAR_HIGH_OFF_WRCH_0         (PCIE_DMA_BASE + 0x380210)
#define DMA_DAR_LOW_OFF_WRCH_0          (PCIE_DMA_BASE + 0x380214)
#define DMA_DAR_HIGH_OFF_WRCH_0         (PCIE_DMA_BASE + 0x380218)
#define DMA_WRITE_DOORBELL_OFF          (PCIE_DMA_BASE + 0x380010)
#define DMA_WRITE_INT_CLEAR_OFF         (PCIE_DMA_BASE + 0x380058)
#define DMA_WRITE_INT_STATUS_OFF		(PCIE_DMA_BASE + 0x38004c)

#define DMA_WRITE_LINKED_LIST_ERR_EN_OFF (PCIE_DMA_BASE + 0x380090)
#define DMA_LLP_LOW_OFF_RDCH_0			 (PCIE_DMA_BASE + 0x38031c)
#define DMA_LLP_HIGH_OFF_RDCH_0			 (PCIE_DMA_BASE + 0x380320)

#endif


