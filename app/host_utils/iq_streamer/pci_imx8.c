/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2024 NXP
*
* PCI driver POC code should be replaced by proper driver like :
* https://github.com/nxp-imx/linux-imx/blob/lf-5.4.y/drivers/pci/controller/dwc/pci-imx6.c#L1793
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <inttypes.h>


#include "vspa_exported_symbols.h"
#include "iq_streamer.h"
#include "stats.h"
#ifndef IMX8DXL
#include "pci_imx8mp.h"
#else
#include "pci_imx8dxl.h"
#endif
#include "l1-trace.h"

void dma_perf_test(void)
{
	return;
}

static int tx_first_dma = 1;
#define PCI_WR_DMA_NB 1
int PCI_DMA_WRITE_transfer(uint32_t ddr_src, uint32_t pci_dst, uint32_t size, uint32_t ant)
{
    volatile uint32_t *reg;
    volatile int dma = 0;
	uint32_t off;
	uint32_t chanStatus = 0;

	l1_trace(L1_TRACE_MSG_DMA_DDR_RD_START, (uint32_t)ddr_src);

	// Do soft reset on start
	if (tx_first_dma) {
		tx_first_dma = 0;
		for (int dma = 0; dma < PCI_WR_DMA_NB; dma++) {
			off = 0x200 * dma;
			reg = (uint32_t *)DMA_WRITE_ENGINE_EN_OFF;
			*reg = 0x0;
			reg = (uint32_t *)DMA_WRITE_ENGINE_EN_OFF;
			*reg = 0x1;
		}
	}

	for (dma = 0; dma < PCI_WR_DMA_NB; dma++) {
		off = 0x200 * dma;
		// Check DMA Halt state
		//reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);
		//if((*reg & 0x60) != 0x60) {
		//	return -1;
		//}
		// Clear previous transfer
		reg = (uint32_t *)DMA_WRITE_INT_CLEAR_OFF;
		*reg = 1 << dma;
		// Configure new transfer
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);
		//*reg = 0x04000008;
		*reg = 0x00000008;
		reg = (uint32_t *)(DMA_TRANSFER_SIZE_OFF_WRCH_0 + off);
		*reg =  size/PCI_WR_DMA_NB;
		reg = (uint32_t *)(DMA_SAR_LOW_OFF_WRCH_0 + off);
		*reg =  ddr_src + dma*(size/PCI_WR_DMA_NB);
		reg = (uint32_t *)(DMA_SAR_HIGH_OFF_WRCH_0 + off);
		*reg =  0x00000000;
		reg = (uint32_t *)(DMA_DAR_LOW_OFF_WRCH_0 + off);
		*reg =  pci_dst + dma*(size/PCI_WR_DMA_NB);
		reg = (uint32_t *)(DMA_DAR_HIGH_OFF_WRCH_0 + off);
		*reg =  0x00000000;
		// start transfer
		reg = (uint32_t *)DMA_WRITE_DOORBELL_OFF;
		*reg =  dma;
	}

	/* wait for completion */
	for (dma = 0; dma < PCI_WR_DMA_NB; dma++) {
		off = 0x200 * dma;
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);
		chanStatus =  *reg;
		while ((chanStatus & 0x40) != 0x40) {
			chanStatus =  *reg;
		}
	}

	l1_trace(L1_TRACE_MSG_DMA_DDR_RD_COMP, (uint32_t)pci_dst);

	/* check status */
	for (dma = 0; dma < PCI_WR_DMA_NB; dma++) {
		off = 0x200 * dma;
		/* wait for completion */
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);
		chanStatus =  *reg;
		if ((chanStatus & 0x60) != 0x60) {
			printf("\n DMA error (0x%08x)\n", chanStatus);
			return -1;
		}
	}

	return 0;
}


static int rx_first_dma = 1;
#define PCI_RD_DMA_NB 1
int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ant)
{
    volatile uint32_t *reg;
    volatile int dma = 0;
	uint32_t off;
	uint32_t chanStatus = 0;

	l1_trace(L1_TRACE_MSG_DMA_DDR_WR_START, (uint32_t)pci_src);

	// Do soft reset on start
	if (rx_first_dma) {
		rx_first_dma = 0;
		for (int dma = 0; dma < PCI_RD_DMA_NB; dma++) {
			off = 0x200 * dma;
			reg = (uint32_t *)DMA_READ_ENGINE_EN_OFF;
			*reg = 0x0;
			reg = (uint32_t *)DMA_READ_ENGINE_EN_OFF;
			*reg = 0x1;
		}
	}

	for (dma = 0; dma < PCI_RD_DMA_NB; dma++) {
		off = 0x200 * dma;
		// Check DMA Halt state
		//reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
		//if((*reg & 0x60) == 0x60) {
		//	return -1;
		//}
		// Clear previous transfer
		reg = (uint32_t *)DMA_READ_INT_CLEAR_OFF;
		*reg = 1 << dma;
		// Configure new transfer
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
//		*reg = 0x04000008 ;
		*reg = 0x00000008;
		reg = (uint32_t *)(DMA_TRANSFER_SIZE_OFF_RDCH_0 + off);
		*reg =  size/PCI_RD_DMA_NB;
		reg = (uint32_t *)(DMA_SAR_LOW_OFF_RDCH_0 + off);
		*reg =  pci_src + dma*size/PCI_RD_DMA_NB;
		reg = (uint32_t *)(DMA_SAR_HIGH_OFF_RDCH_0 + off);
		*reg =  0x00000000;
		reg = (uint32_t *)(DMA_DAR_LOW_OFF_RDCH_0 + off);
		*reg =  ddr_dst + dma*size/PCI_RD_DMA_NB;
		reg = (uint32_t *)(DMA_DAR_HIGH_OFF_RDCH_0 + off);
		*reg =  0x00000000;
		reg = (uint32_t *)DMA_READ_DOORBELL_OFF;
		*reg =  dma;
	}

	/* wait for completion */
	for (dma = 0; dma < PCI_RD_DMA_NB; dma++) {
		off = 0x200 * dma;
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
		chanStatus =  *reg;
		while ((chanStatus & 0x40) != 0x40) {
			chanStatus =  *reg;
		}
	}

	l1_trace(L1_TRACE_MSG_DMA_DDR_WR_COMP, (uint32_t)ddr_dst);

	/* check status */
	for (dma = 0; dma < PCI_RD_DMA_NB; dma++) {
		off = 0x200 * dma;
		/* wait for completion */
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
		chanStatus =  *reg;
		if ((chanStatus & 0x60) != 0x60) {
			printf("\n DMA error (0x%08x)\n", chanStatus);
			return -1;
		}
	}

	return 0;
}


#if 0
/*
## clear
bin2mem -f 0pad_2MB.bin -a 0x92400000

## build Linked List Mode in DDR
## re-using first 2MB scratch buffer)
## 3 elements : (CYC_COUNTER_MSB/LSB) 8B + VSPA incl. DMEM 1MB + (CYC_COUNTER_MSB/LSB) 8B

memtool -32  0x92400000=0x00000001
memtool -32  0x92400004=0x00000008
memtool -32  0x92400008=0x19000098
memtool -32  0x9240000C=0
memtool -32  0x92400010=0x92400100
memtool -32  0x92400014=0
memtool -32  0x92400018=0x00000001
memtool -32  0x9240001c=0x8000
memtool -32  0x92400020=0x1F400000
memtool -32  0x92400024=0
memtool -32  0x92400028=0x92400120
memtool -32  0x9240002c=0
memtool -32  0x92400030=0x00000009
memtool -32  0x92400034=0x00000008
memtool -32  0x92400038=0x19000098
memtool -32  0x9240003c=0
memtool -32  0x92400040=0x92400110
memtool -32  0x92400044=0
memtool -32  0x92400048=0x00000006
memtool -32  0x9240004c=0
memtool -32  0x92400050=0x92400000
memtool -32  0x92400054=0

## init DMA channel 0
memtool -32  0x33b8002c=0			// DMA_READ_ENGINE_EN_OFF
memtool -32  0x33b8002c=1			// DMA_READ_ENGINE_EN_OFF
memtool -32  0x33b800A8=0			// DMA_READ_INT_CLEAR_OFF
memtool -32  0x33b80090=0			// DMA_WRITE_LINKED_LIST_ERR_EN_OFF
memtool -32  0x33b80300=0x04000300	// DMA_CH_CONTROL1_OFF_RDCH_0
memtool -32  0x33b8031c=0x92400000	// DMA_LLP_LOW_OFF_RDCH_0 - DMA Read Linked List Pointer Low Register.
memtool -32  0x33b80320=0			// DMA_LLP_HIGH_OFF_RDCH_0 - DMA Read Linked List Pointer High Register.
memtool -32  0x33b80018=0			// DMA_WRITE_CHANNEL_ARB_WEIGHT_LOW_OFF
memtool -32  0x33b80030=0			// DMA_READ_DOORBELL_OFF

## ouput :
memtool -32 0x92400000 100
*/

int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ant)
{
    volatile uint32_t *reg;
    volatile int cnt = 0;
    volatile int dma = 0;
    volatile int can_setup_dma[2] = {0, 0};
	uint32_t off;
	uint32_t chanStatus = 0;

	off = 0x200 * dma;


	if (rx_first_dma)
		{
		reg = (uint32_t *)DMA_READ_ENGINE_EN_OFF;
		*reg = 0x0;

		reg = (uint32_t *)DMA_READ_ENGINE_EN_OFF;
		*reg = 0x1;

		reg = (uint32_t *)DMA_READ_INT_MASK_OFF;
		*reg = 0x0;

		rx_first_dma = 0;

		can_setup_dma[dma] = 1;

		}
	else
		{
		reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
		if ((*reg & 0x60) == 0x60) {
			can_setup_dma[dma] = 1;
			}
		}

	/* wait for completion */
    //while((*reg & 0x60) != 0x60) {};

    if (!can_setup_dma[dma]) {
		// DMA busy
	return -1;
    }
	/* build PCI DMA desc list in memory v_scratch_ddr_addr */

	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x0) = 0x00000001;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x4) = 0x00000008;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x8) = 0x19000098;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0xC) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x10) = 0x92400100;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x14) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x18) = 0x00000001;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x1c) = 0x8000;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x20) = pci_src;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x24) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x28) = ddr_dst;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x2c) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x30) = 0x00000009;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x34) = 0x00000008;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x38) = 0x19000098;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x3c) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x40) = 0x92400110;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x44) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x48) = 0x00000006;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x4c) = 0;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x50) = 0x92400000;
	*(uint32_t *)((uint64_t)v_scratch_ddr_addr + 0x54) = 0;



	reg = (uint32_t *)DMA_READ_INT_CLEAR_OFF;
	*reg = 1 << dma;

	reg = (uint32_t *)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
	*reg = 0x04000300;

	reg = (uint32_t *)(DMA_LLP_LOW_OFF_RDCH_0 + off);
	*reg = SCRATCH_ADDR;

	reg = (uint32_t *)(DMA_LLP_HIGH_OFF_RDCH_0 + off);
	*reg = 0;

	reg = (uint32_t *)DMA_READ_DOORBELL_OFF;
	*reg =  dma;

	/* wait for completion */
	chanStatus =  *reg;
    while ((chanStatus & 0x40) != 0x40) {
		chanStatus =  *reg;
	}

	if ((chanStatus & 0x60) != 0x60) {
		printf("\n DMA error (0x%08x)\n", chanStatus);
		return -1;
	}

    cnt += 1;

	return 0;
}
#endif
