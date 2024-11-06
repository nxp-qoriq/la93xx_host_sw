/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2024 NXP
*
* PCI driver POC code should be replaced by proper driver like :
* https://github.com/nxp-imx/linux-imx/blob/lf-5.4.y/drivers/pci/controller/dwc/pci-imx6.c#L1793
*
*/

#ifdef __M7__
#include "fsl_common.h"
#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <inttypes.h>
#endif

#include "imx8-host.h"

#ifndef _STANDALONE_
#include "vspa_exported_symbols.h"
#include "l1-trace.h"
#include "iqmod_rx.h"
#include "iqmod_tx.h"
#include "stats.h"
#include "imx_edma_api.h"
#else
#define l1_trace(a,b){};
#endif

#ifndef IMX8DXL
#include "pci_imx8mp.h"
#else
#include "pci_imx8dxl.h"
#endif



void dma_perf_test(void)
{
	return;
}

static int tx_pending_dma = 0;
extern uint32_t txcount;
#ifndef _STANDALONE_
extern uint32_t local_TX_total_produced_size;
#else
uint32_t g_error=0;
#endif
int PCI_DMA_WRITE_completion(uint32_t nbchan)
{
    volatile uint32_t *reg;
    int dma = 0;
	uint32_t ret = 0;
	uint32_t error = 0;

	if(!tx_pending_dma)
		return 1;
	
	/* check completion */
	for (dma = 0; dma < nbchan; dma++) {
		ret = pci_dma_completed(dma + 1, 0);
		if (ret == -1) {
			/* still running */
			return 0;
		}

		if (ret == -2)
			error = 1;
	}

	tx_pending_dma=0;
	txcount++;
#ifndef _STANDALONE_
	host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]+=error;
	l1_trace(L1_TRACE_MSG_DMA_DDR_RD_COMP, txcount);
	host_stats->tx_stats[STAT_EXT_DMA_DDR_RD]=txcount;
	*v_TX_total_produced_size=local_TX_total_produced_size;
#else
	g_error+=error;
#endif

	return 1;
}


static int tx_first_dma = 1;
int PCI_DMA_WRITE_transfer(uint32_t ddr_src, uint32_t pci_dst, uint32_t size,uint32_t nbchan)
{
    volatile uint32_t *reg;
    volatile int dma = 0;

	// Do soft reset on start
	if (tx_first_dma) {
		tx_first_dma = 0;
		for (int dma = 0; dma < nbchan; dma++) {
			pci_dma_tx_reset();
		}
	}
	
	//while (!PCI_DMA_WRITE_completion(nbchan)) {};

	l1_trace(L1_TRACE_MSG_DMA_DDR_RD_START, (uint32_t)ddr_src);

	for (dma = 0; dma < nbchan; dma++)
		pci_dma_write(ddr_src, pci_dst, size / nbchan, dma + 1);

	/* mark dma running */
	tx_pending_dma=1;

	while (!PCI_DMA_WRITE_completion(nbchan)) {};

	return 0;
}


static int rx_pending_dma = 0;
extern uint32_t rxcount;
#ifndef _STANDALONE_
extern uint32_t local_RX_total_consumed_size;
#endif
int PCI_DMA_READ_completion(uint32_t nbchan)
{
    volatile uint32_t *reg;
    int dma = 0;
	uint32_t ret = 0;
	uint32_t error=0;

	if(!rx_pending_dma)
		return 1;

	/* wait for completion */
	for (dma = 0; dma < nbchan; dma++) {
		ret = pci_dma_completed(dma + 1, 1);
		if (ret == -1) {
			/* still running */
			return 0;
		}

		if (ret == -2)
			error = 1;
	}
	
	rx_pending_dma=0;
	rxcount++;
#ifndef _STANDALONE_
	l1_trace(L1_TRACE_MSG_DMA_DDR_WR_COMP, rxcount);
	host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]+=error;
	host_stats->rx_stats[0][STAT_EXT_DMA_DDR_WR]=rxcount;
	*v_RX_total_consumed_size = local_RX_total_consumed_size;
#else
	g_error+=error;
#endif

	return 1;
}


static int rx_first_dma = 1;
int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size,uint32_t nbchan)
{
    volatile uint32_t *reg;
    volatile int dma = 0;

	// Do soft reset on start
	if (rx_first_dma) {
		rx_first_dma = 0;
		for (int dma = 0; dma < nbchan; dma++) {
			pci_dma_rx_reset();
		}
	}
	
	//while (!PCI_DMA_READ_completion(nbchan)) {};

	l1_trace(L1_TRACE_MSG_DMA_DDR_WR_START, (uint32_t)pci_src);

	for (dma = 0; dma < nbchan; dma++)
		pci_dma_read(pci_src, ddr_dst, size / nbchan, dma + 1);

	/* mark dma running */
	rx_pending_dma=1;

	while (!PCI_DMA_READ_completion(nbchan)) {};

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

int PCI_DMA_READ_transfer(uint32_t pci_src, uint32_t ddr_dst, uint32_t size)
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
		//PRINTF("\n DMA error (0x%08x)\n", chanStatus);
		return -1;
	}

    cnt += 1;

	return 0;
}
#endif
