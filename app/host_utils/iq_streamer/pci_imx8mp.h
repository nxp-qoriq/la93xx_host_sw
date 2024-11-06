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

#endif


