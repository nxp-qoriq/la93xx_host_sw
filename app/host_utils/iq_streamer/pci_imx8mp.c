/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2022 NXP
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
#include "pci_imx8mp.h"


static int tx_first_dma=1;
int PCI_DMA_WRITE_transfer(uint32_t ddr_src,uint32_t pci_dst,uint32_t size,uint32_t ant)
{
    volatile uint32_t *reg;
    volatile int cnt = 0;
    volatile int dma = 0;
    volatile int can_setup_dma[2]={0,0};
	uint32_t off;
	uint32_t chanStatus=0;

	GET_VSPA_CYCLE_COUNT(cycle_count_xfer_start,ant,cycle_count_index[ant]) 
	
	off = 0x200 * dma;

	if(tx_first_dma)
		{
		reg = (uint32_t*)DMA_WRITE_ENGINE_EN_OFF; 
		*reg = 0x0;

		reg = (uint32_t*)DMA_WRITE_ENGINE_EN_OFF; 
		*reg = 0x1;
		
		reg = (uint32_t*)DMA_WRITE_INT_MASK_OFF; 
		*reg = 0x0;

		tx_first_dma=0;

		can_setup_dma[dma] = 1;

		}
	else
		{
		reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);
		if((*reg & 0x60) == 0x60) {
			can_setup_dma[dma] = 1;
			}
		}	

	/* wait for completion */
    //while((*reg & 0x60) != 0x60) {};

    if(!can_setup_dma[dma]) {
		// DMA busy
        return -1;
    }

	
    reg = (uint32_t*)DMA_WRITE_INT_CLEAR_OFF;
    *reg = 1 << dma;
    
    reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_WRCH_0 + off); 
    *reg = 0x04000008;
    //*reg = 0x00000008 ;//| (1<<25);
    //*reg = 0x00000000 ;//| (1<<25);
	
    reg = (uint32_t*)(DMA_TRANSFER_SIZE_OFF_WRCH_0 + off); 
    //*reg =  0x40000;
    *reg =  size;
    
    reg = (uint32_t*)(DMA_SAR_LOW_OFF_WRCH_0 + off); 
    //*reg = 0x1C010A00;
    *reg =  ddr_src; 
    //*reg =  0x1c000000;
    //*reg = 0x1f000000;
    reg = (uint32_t*)(DMA_SAR_HIGH_OFF_WRCH_0 + off); 
    *reg =  0x00000000;
    reg = (uint32_t*)(DMA_DAR_LOW_OFF_WRCH_0 + off); 
    *reg =  pci_dst;
    reg = (uint32_t*)(DMA_DAR_HIGH_OFF_WRCH_0 + off); 
    *reg =  0x00000000;
    
    reg = (uint32_t*)DMA_WRITE_DOORBELL_OFF; 
    *reg =  dma;
	
    can_setup_dma[dma] = 0; 
	
    reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_WRCH_0 + off);

	/* wait for completion */
	chanStatus=*reg;
    while((chanStatus & 0x40) != 0x40) {
		chanStatus=*reg;
	}
	
	GET_VSPA_CYCLE_COUNT(cycle_count_xfer_done,ant,cycle_count_index[ant]) 

	if((chanStatus & 0x60) != 0x60) {
		printf("\n DMA error (0x%08x)\n",chanStatus);
		return -1;
	}    
    cnt += 1;
	
	return 0;
}

static int rx_first_dma=1;
int PCI_DMA_READ_transfer(uint32_t pci_src,uint32_t ddr_dst,uint32_t size,uint32_t ant)
{
    volatile uint32_t *reg;
    volatile int cnt = 0;
    volatile int dma = 0;
    volatile int can_setup_dma[2]={0,0};
	uint32_t off;
	uint32_t chanStatus=0;

	GET_VSPA_CYCLE_COUNT(cycle_count_xfer_start,ant,cycle_count_index[ant]) 
	
	off = 0x200 * dma;

	if(rx_first_dma)
		{
		reg = (uint32_t*)DMA_READ_ENGINE_EN_OFF; 
		*reg = 0x0;

		reg = (uint32_t*)DMA_READ_ENGINE_EN_OFF; 
		*reg = 0x1;
		
		reg = (uint32_t*)DMA_READ_INT_MASK_OFF; 
		*reg = 0x0;

		rx_first_dma=0;

		can_setup_dma[dma] = 1;

		}
	else
		{
		reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);
		if((*reg & 0x60) == 0x60) {
			can_setup_dma[dma] = 1;
			}
		}	

	/* wait for completion */
    //while((*reg & 0x60) != 0x60) {};

    if(!can_setup_dma[dma]) {
		// DMA busy
        return -1;
    }

	
	reg = (uint32_t*)DMA_READ_INT_CLEAR_OFF;
	*reg = 1 << dma;
	
	reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_RDCH_0 + off); 
	//*reg = 0x04000008;
	*reg = 0x04000008 ;//| (1<<25);
	
	reg = (uint32_t*)(DMA_TRANSFER_SIZE_OFF_RDCH_0 + off); 
	//*reg =  0x40000;
	*reg =  size;
	reg = (uint32_t*)(DMA_SAR_LOW_OFF_RDCH_0 + off); 
	//*reg = 0x1C010A00;
	*reg =  pci_src; 
	//*reg =  0x1c000000;
	//*reg = 0x1f000000;
	reg = (uint32_t*)(DMA_SAR_HIGH_OFF_RDCH_0 + off); 
	*reg =  0x00000000;
	reg = (uint32_t*)(DMA_DAR_LOW_OFF_RDCH_0 + off); 
	*reg =  ddr_dst;
	reg = (uint32_t*)(DMA_DAR_HIGH_OFF_RDCH_0 + off); 
	*reg =  0x00000000;
		
	reg = (uint32_t*)DMA_READ_DOORBELL_OFF; 
	*reg =  dma;
	
    can_setup_dma[dma] = 0; 
	
    reg = (uint32_t*)(DMA_CH_CONTROL1_OFF_RDCH_0 + off);

	/* wait for completion */
	chanStatus=*reg;
    while((chanStatus & 0x40) != 0x40) {
		chanStatus=*reg;
	}
	
	GET_VSPA_CYCLE_COUNT(cycle_count_xfer_done,ant,cycle_count_index[ant]) 

	if((chanStatus & 0x60) != 0x60) {
		printf("\n DMA error (0x%08x)\n",chanStatus);
		return -1;
	}    
    cnt += 1;
	
	return 0;
}
