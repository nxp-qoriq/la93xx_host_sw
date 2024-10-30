/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>
#include <semaphore.h>
#include <signal.h>
#include <argp.h>

//#include "kpage_ncache_api.h"

#ifndef IMX8DXL
#include "pci_imx8mp.h"
#else
#include "pci_imx8dxl.h"
#endif

#include "la9310_regs.h"
#include "l1-trace.h"

#ifdef IQMOD_RX_2R
#include "vspa_exported_symbols_2R.h"
#else
#ifdef IQMOD_RX_4R
#include "vspa_exported_symbols_4R.h"
#else
#include "vspa_exported_symbols.h"
#endif
#endif

#include "iqmod_rx.h"
#include "iqmod_tx.h"
#include "stats.h"
#include "iq_streamer.h"
#include "imx8-host.h"
#include "vspa_dmem_proxy.h"


void la9310_hexdump(const void *ptr, size_t sz);
void print_vspa_stats(void);
void monitor_vspa_stats(void);
void print_vspa_trace(void);
void print_m7_trace(void);


volatile uint32_t running;

sem_t thread_sem;
void *process_ant_tx(void *arg);
void *process_ant_rx(void *arg);
void *process_ant_tx_vspa_ext_dma(void *arg);
void *process_ant_tx_streaming_tcm(void *arg);
void *process_ant_tx_streaming_ddr(void *arg);
void *process_ant_rx_vspa_ext_dma(void *arg);
void *process_ant_rx_streaming_tcm(void *arg);
void *process_ant_rx_streaming_ddr(void *arg);

uint32_t *_VSPA_A011455;
uint32_t *v_iqflood_ddr_addr;
uint32_t *v_scratch_ddr_addr;
uint32_t *v_ocram_addr;


uint32_t p_la9310_outbound_base;
uint32_t *BAR0_addr;
uint32_t *BAR1_addr;
uint32_t *BAR2_addr;
uint32_t *PCIE1_addr;
t_stats *host_stats;

uint32_t ddr_file_addr = 0;
uint32_t ddr_file_size = 0;

uint32_t isVspaExtPciDMA = 1;
uint32_t isM7offload=0;
uint32_t isStreamingTcmFifo = 0;
uint32_t ep_tcm_fifo_address = 0;
uint32_t ep_tcm_fifo_size = 0;
uint32_t ddr_fifo_addr=0;
uint32_t ddr_fifo_size=0;
uint32_t ddr_rd_dma_xfr_size=TX_DDR_STEP;
uint32_t ddr_wr_dma_xfr_size=RX_DDR_STEP*RX_COMPRESS_RATIO_PCT/100;

void print_host_trace(void);

int map_physical_regions(void)
{
	int devmem_fd, i;
	//void *scr_p;
	//uint32_t mapsize;
	//uint64_t k_page;

	/*
	* map memory regions
	*/

	devmem_fd = open("/dev/mem", O_RDWR);
	if (-1 == devmem_fd) {
		perror("/dev/mem open failed");
		return -1;
	}

	v_ocram_addr = mmap(NULL, OCRAM_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, OCRAM_ADDR);
	if (v_ocram_addr == MAP_FAILED) {
		perror("Mapping v_ocram_addr buffer failed\n");
		return -1;
	}

	/* use first ocram 256 bytes for dmem proxy region followed by M7 trace */
	host_stats = (t_stats *)v_ocram_addr;

//	printf("\n map iqflood_ddr :");
//	la9310_hexdump(v_iqflood_ddr_addr,64);

	v_iqflood_ddr_addr = mmap(NULL, IQFLOOD_BUF_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, IQFLOOD_BUF_ADDR);
	if (v_iqflood_ddr_addr == MAP_FAILED) {
		perror("Mapping v_iqflood_ddr_addr buffer failed\n");
		return -1;
	}
	
//	printf("\n map iqflood_ddr :");
//	la9310_hexdump(v_iqflood_ddr_addr,64);

	v_scratch_ddr_addr = mmap(NULL, SCRATCH_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, SCRATCH_ADDR);
	if (v_scratch_ddr_addr == MAP_FAILED) {
		perror("Mapping v_scratch_ddr_addr buffer failed\n");
		return -1;
	}

    *(volatile uint32_t *)(v_scratch_ddr_addr) = 0;

//	printf("\n map v_scratch_ddr_addr :");
//	la9310_hexdump(v_scratch_ddr_addr,64);

	BAR0_addr = mmap(NULL, BAR0_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR0_ADDR);
	if (BAR0_addr == MAP_FAILED) {
		perror("Mapping BAR0_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR0_addr :");
//	la9310_hexdump(BAR0_addr,64);

	BAR1_addr = mmap(NULL, BAR1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR1_ADDR);
	if (BAR1_addr == MAP_FAILED) {
		perror("Mapping BAR1_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR1_addr :");
//	la9310_hexdump(BAR1_addr,64);

	BAR2_addr = mmap(NULL, BAR2_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR2_ADDR);
	if (BAR2_addr == MAP_FAILED) {
		perror("Mapping BAR2_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR2_addr :");
//	la9310_hexdump(BAR2_addr,64);
#if 0
	scr_p = BAR2_addr;
	mapsize = BAR2_SIZE;
	k_page = (uint64_t)scr_p & ~0xfff;
//	while(k_page<(uint64_t)scr_p+mapsize) {
	mark_kpage_ncache(k_page);
	k_page += 0x1000;
//	}
#endif


	PCIE1_addr = mmap(NULL, IMX8MP_PCIE1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, IMX8MP_PCIE1_ADDR);
	if (PCIE1_addr == MAP_FAILED) {
		perror("Mapping PCIE1_addr buffer failed\n");
		return -1;
	}
//	printf("\n map PCIE1_addr :");
//	la9310_hexdump(PCIE1_addr,64);

	/* search outbound windows mapping iqflood */
	for (i = 0; i <= 3; i++) {
		*(volatile uint32_t *)((uint64_t)BAR0_addr + IATU_VIEWPORT_OFF) = i;
		if (*(volatile uint32_t *)((uint64_t)BAR0_addr + ATU_LWR_TARGET_ADDR_OFF_INBOUND_0) == IQFLOOD_BUF_ADDR)
		{
			p_la9310_outbound_base =  *(volatile uint32_t *)((uint64_t)BAR0_addr + ATU_LWR_BASE_ADDR_OFF_OUTBOUND_0);
			break;
		}
	}
	if (i <= 3) {
		printf("la9310 pci outbound to ddr iqflood found : 0x%08x", p_la9310_outbound_base);
	} else {
		printf("la9310 pci outbound to iqflood (0x%08x) not found\n", IQFLOOD_BUF_ADDR);
		return -1;
	}

	close(devmem_fd);

	return 0;
}

/*
 *  terminate_process() SIGINT,SIGTERM handler
 *  sigusr1_process() SIGUSR1 handler
 *  user may issue "kill -USR1 <PID>" to trigger usr1
 */
void terminate_process(int sig)
{
     running = 0;
      *v_RX_ext_dma_enabled = 0;
}

void sigusr1_process(int sig)
{
    fflush(stdout);
	print_host_trace();
}

void print_cmd_help(void)
{
    fprintf(stderr, "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    fprintf(stderr, "\n|    iq_streamer");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| ./iq_streamer -s ");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| Trace");
    fprintf(stderr, "\n|\t-h	help");
    fprintf(stderr, "\n|\t-a    <padd> <size>  Buffer in DDR (vspa ext dma)");
    fprintf(stderr, "\n|\t-F    <padd> <size>  Fifo in DDR");
    fprintf(stderr, "\n|\t-T    <padd> <size>  Fifo in TCM");
    fprintf(stderr, "\n|\t-s    <ddr_rd_dma_xfr_size> <ddr_wr_dma_xfr_size>");
    fprintf(stderr, "\n|\t-t	Tx streaming only");
    fprintf(stderr, "\n|\t-r	Rx streaming only");
    fprintf(stderr, "\n|\t-m	monitor stats");
    fprintf(stderr, "\n|\t-d	dump vspa trace");
    fprintf(stderr, "\n|\t-D	dump M7 trace");
    fprintf(stderr, "\n|\t-c	clear host stats");
    fprintf(stderr, "\n|\t-v	version");
    fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
//VSPA_EXPORT(ddr_rd_dma_xfr_size)
//VSPA_EXPORT(ddr_wr_dma_xfr_size)

int main(int argc, char *argv[])
{
    int32_t c,i;
	command_e command = 0;
	struct stat buffer;
    int32_t forceFifoSize=0;


	if(argc<=1){
		print_cmd_help();
		exit(1);
	}
		

   /* command line parser */
   while ((c = getopt(argc, argv, "hfrts:a:mdvxF:T:DMc")) != EOF)
   {
		switch (c) {
			case 'h': // help
				print_cmd_help();
				exit(1);
			break;
			case 'v':
				printf("%s", IQ_STREAMER_VERSION);
				exit(1);
			case 'f':
				command = OP_TX_RX;
			break;
			case 't':
				command = OP_TX_ONLY;
			break;
			case 'r':
				command = OP_RX_ONLY;
			break;
			case 'a':
				ddr_file_addr = strtoull(argv[optind-1], 0, 0);
				ddr_file_size = strtoul(argv[optind],0, 0);
			break;
			case 'T':	
				isStreamingTcmFifo=1;			
				isVspaExtPciDMA=0;			
				ep_tcm_fifo_address = strtoull(argv[optind-1], 0, 0);
				ep_tcm_fifo_size = strtoul(argv[optind],0, 0);
			break;
			case 'F':
				isStreamingTcmFifo=0;			
				isVspaExtPciDMA=0;			
				ddr_fifo_addr = strtoull(argv[optind-1], 0, 0);
				ddr_fifo_size = strtoul(argv[optind],0, 0);
			break;
			case 'M':
				isM7offload=1;
			break;
			case 'm':
				command = OP_MONITOR;
			break;
			case 'd':
				command = OP_DUMP_TRACE;
			break;
			case 'D':
				command=OP_DUMP_TRACE_M7;
			break;
			case 'x':
				command = OP_DMA_PERF;
			break;
			case 'c':
				command = OP_CLEAR_STATS;
			break;
			case 's':
			    forceFifoSize=1;
				ddr_rd_dma_xfr_size=strtoull(argv[optind-1], 0, 0);
				ddr_wr_dma_xfr_size=strtoull(argv[optind],0, 0);
			break;
			default:
				print_cmd_help();
				exit(1);
			break;

		}
	}

	if (stat("/sys/shiva/shiva_status", &buffer) != 0) {
		perror("la9310 shiva driver not started");
		exit(EXIT_FAILURE);
	}

	running = 1;

	/*
	* map memory regions
	*/
	if (map_physical_regions()) {
		perror("map_physical_regions failed:");
		exit(EXIT_FAILURE);
	}

	/*
	 *   start stream Tx IQ data thread
	 */
	
	if (forceFifoSize) {
		*v_ddr_rd_dma_xfr_size=ddr_rd_dma_xfr_size;
		*v_ddr_wr_dma_xfr_size=ddr_wr_dma_xfr_size;
	}

	if (command <= OP_TX_RX) {
		signal(SIGINT, terminate_process);
		signal(SIGTERM, terminate_process);
		signal(SIGUSR1, sigusr1_process);
	}

	if (command == OP_CLEAR_STATS) {
		for (i = 0; i < s_g_stats/4; i++) {
			//dccivac((uint32_t*)(host_stats)+i);
			*((uint32_t*)(host_stats)+i)=0;
		}
	}

	if (command == OP_TX_ONLY) {
		if(isVspaExtPciDMA)
			process_ant_tx_vspa_ext_dma(NULL);
		else if (isStreamingTcmFifo)
			process_ant_tx_streaming_tcm(NULL);
		else 
			perror("not supported");
	}

	if (command == OP_RX_ONLY) {
		if(isVspaExtPciDMA)
			process_ant_rx_vspa_ext_dma(NULL);
		else if (isStreamingTcmFifo)
			perror("not supported");
		else 
			perror("not supported");
	}

	if (command == OP_STATS) {
		print_vspa_stats();
	}

	if (command == OP_MONITOR) {
		monitor_vspa_stats();
	}

	if (command == OP_DUMP_TRACE) {
		print_vspa_trace();
	}

	if(command==OP_DUMP_TRACE_M7){
		print_m7_trace();
	}

	if (command == OP_DMA_PERF) {
		dma_perf_test();
	}

	return 0;
}


/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions 						*/
//VSPA_EXPORT(DDR_rd_base_address)
//VSPA_EXPORT(DDR_rd_size)
//VSPA_EXPORT(TX_total_produced_size)
//VSPA_EXPORT(TX_total_consumed_size)
//VSPA_EXPORT(output_buffer)
//VSPA_EXPORT(TX_ext_dma_enabled)
//VSPA_EXPORT(DDR_rd_load_start_bit_update)
//VSPA_EXPORT(DDR_rd_start_bit_update)

uint32_t txcount=0;
uint32_t local_TX_total_produced_size=0;
uint32_t local_TX_total_consumed_size=0;
uint32_t local_TX_ext_dma_enabled=0;
uint32_t local_DDR_rd_load_start_bit_update=0;

void *process_ant_tx_vspa_ext_dma(void *arg)
{
	int ret;
	uint32_t busy_size=0;
	uint32_t empty_size=0;
	uint32_t dmem_dst_offset=0;
	uint32_t ddr_start_address=0;
	uint32_t ddr_file_size=0;
	uint32_t ddr_rd_offset=0;
	uint32_t loopcount=0;
	

	if(isM7offload){
	    // Advertise VSPA streamer has started
	    *v_TX_ext_dma_enabled=HOST_DMA_M7;
		while (running){};
	} else
	{
	    // Advertise VSPA streamer has started
	    *v_TX_ext_dma_enabled=HOST_DMA_LINUX;
restart_tx:
		// Wait tx streaming to start
		printf("\n TX : Waiting _DDR_base_address != 0xdeadbeef \n");
		fflush(stdout);
		dmem_dst_offset=0;
		ddr_rd_offset=0;
		ddr_start_address=*v_DDR_rd_base_address;
		while ((ddr_start_address==0xdeadbeef)&&running){
			//microsleep();
			ddr_start_address=*v_DDR_rd_base_address;
			};
		ddr_file_size=*v_DDR_rd_size;

		// Check pointers are cleared
		local_DDR_rd_load_start_bit_update=*v_DDR_rd_load_start_bit_update;
		local_TX_total_consumed_size=*v_TX_total_consumed_size;
		local_TX_total_produced_size=*v_TX_total_produced_size;
		
		// start feeding buffers
		while (running){
			// wait room in dmem buffers
			do {
				loopcount++;
				// slow check stop event
				if(loopcount>500){
					loopcount=0;
					ddr_start_address=*v_DDR_rd_base_address;
					if(ddr_start_address==0xdeadbeef){
						goto restart_tx;
					}
				}
				// DMA perf (don't wait for consumer)
				if(local_DDR_rd_load_start_bit_update!=0){
					break;
				}
				// check DMA completion
				//PCI_DMA_WRITE_completion(NB_WCHAN);
				// normal mode check room for new transfer
				local_TX_total_consumed_size=*v_TX_total_consumed_size;
				busy_size= local_TX_total_produced_size - local_TX_total_consumed_size; 
				empty_size= (TX_NUM_BUF*TX_DDR_STEP) - busy_size; 
				if(empty_size >= EXT_DMA_TX_DDR_STEP){
					// ready for next transfer
					local_TX_total_produced_size += EXT_DMA_TX_DDR_STEP;
					break;
				}
			} while(1);

			// xfer one buffer
			ret= PCI_DMA_WRITE_transfer(IQFLOOD_BUF_ADDR +(ddr_start_address-p_la9310_outbound_base)+ddr_rd_offset,(uint32_t)(p_output_buffer) + dmem_dst_offset,EXT_DMA_TX_DDR_STEP,NB_WCHAN);
			if(ret){
				host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]++;
			}

			// update pointers
			ddr_rd_offset+= EXT_DMA_TX_DDR_STEP;
			if(ddr_rd_offset>=ddr_file_size) {
				ddr_rd_offset=0;
			} 
			dmem_dst_offset+=EXT_DMA_TX_DDR_STEP;
			if(dmem_dst_offset>=(TX_NUM_BUF*TX_DDR_STEP)) {
				dmem_dst_offset=0;
			}
		}
	}

	// Advertise VSPA streamer stopped
	*v_TX_ext_dma_enabled=HOST_DMA_OFF;

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}

void *process_ant_tx_streaming_tcm(void *arg)
{
	int ret;
	uint32_t busy_size = 0;
	uint32_t empty_size = 0;
	uint32_t tcm_fifo_addr = 0;
	uint32_t tcm_fifo_size = 0;
	uint32_t tcm_dst_offset = 0;
	uint32_t ddr_rd_offset = 0;
	uint32_t load_dma_test=0;
	uint32_t loopcount=0;

	
	// Wait tx streaming to start
	printf("\n TX : start iq_streamer in TCM @0x0x%08x\n",ep_tcm_fifo_address);
	fflush(stdout);
	tcm_dst_offset = 0;
	ddr_rd_offset = 0;

	if (ddr_file_addr == 0) {
		printf("\n invalid ddr_file_addr\n");
		return 0;
	}
	if (ddr_file_size == 0) {
		printf("\n invalid ddr_file_size\n");
		return 0;
	}

restart_tx_tcm:

	tcm_fifo_addr = (uint64_t)BAR2_ADDR + ep_tcm_fifo_address - TCM_START;
	tcm_fifo_size = ep_tcm_fifo_size;


	// Check pointers are cleared
	local_TX_total_consumed_size=*v_TX_total_consumed_size;
	local_TX_total_produced_size=*v_TX_total_produced_size;
	load_dma_test=*v_DDR_rd_load_start_bit_update;
	// start feeding buffers
	while (running) {
		// wait room in TCM TX FIFO buffers
		do {
			// slow check stop event
			if(loopcount>500){
				loopcount=0;
				if(!(*v_DDR_rd_start_bit_update)){
				// stopped ?
					goto restart_tx_tcm;
				}
			}
			if (load_dma_test) {
			// DMA perf test
				break;
			}
			// check DMA completion
			//PCI_DMA_WRITE_completion(NB_WCHAN);
			// normal mode check room for new transfer
			local_TX_total_consumed_size =  *v_TX_total_consumed_size;
			busy_size = local_TX_total_produced_size - local_TX_total_consumed_size;
			empty_size = tcm_fifo_size - busy_size; // fifo size - data in tcm
			if (empty_size >= EXT_DMA_TX_DDR_STEP) {
				// ready for next transfer
				local_TX_total_produced_size += EXT_DMA_TX_DDR_STEP;
				break;
			}
			if ((empty_size == tcm_fifo_size) && !local_TX_total_produced_size) {
				((t_stats *)(v_g_stats))->tx_stats[ERROR_EXT_DMA_DDR_RD_UNDERRUN] += 1;
			}
			// check dma complettion
		} while (running);

		ret = PCI_DMA_WRITE_transfer(ddr_file_addr + ddr_rd_offset, tcm_fifo_addr + tcm_dst_offset, EXT_DMA_TX_DDR_STEP,NB_WCHAN);

		if (ret) {
			printf("\n!! DMA not ready !!\n");
			break;
		}

		ddr_rd_offset += EXT_DMA_TX_DDR_STEP;
		if (ddr_rd_offset >= ddr_file_size) {
			ddr_rd_offset = 0;
		}
		tcm_dst_offset += EXT_DMA_TX_DDR_STEP;
		if (tcm_dst_offset >= tcm_fifo_size) {
			tcm_dst_offset = 0;
		}
	}

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}

void *process_ant_tx_streaming_ddr(void *arg)
{
	return 0;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions						*/
//VSPA_EXPORT(DDR_wr_base_address)
//VSPA_EXPORT(DDR_wr_size)
//VSPA_EXPORT(RX_total_produced_size)
//VSPA_EXPORT(RX_total_consumed_size)
//VSPA_EXPORT(rx_ch_context)
//VSPA_EXPORT(input_buffer)
//VSPA_EXPORT(RX_ext_dma_enabled)
//VSPA_EXPORT(DDR_wr_load_start_bit_update)
//VSPA_EXPORT(DDR_wr_CMP_enable)

uint32_t local_RX_total_produced_size;
uint32_t local_RX_total_consumed_size;
uint32_t local_DDR_wr_load_start_bit_update;

uint32_t rxcount=0;

#ifdef v_RX_total_produced_size /* only 1T1R */
void *process_ant_rx_vspa_ext_dma(void *arg)
{
	int ret;
	uint32_t data_size = 0;
	uint32_t dmem_src_offset = 0;
	uint32_t ddr_start_address = 0;
	uint32_t ddr_file_size = 0;
	uint32_t ddr_wr_offset = 0;
	//uint32_t xfr_size;
	uint32_t loopcount=0;

	// !! this workaround is needed otherwise VSPA gets stuck ( dmac_is_available()/dmac_is_complete() doesn't work)
	// !! unless issue "ccs::config_chain {la9310 dap}" on ccs
	//_VSPA_A011455 = (uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + 0x24);
	//*_VSPA_A011455 |= 0x10;

	if(isM7offload){
		// Advertise VSPA streamer has started
		*v_RX_ext_dma_enabled = HOST_DMA_M7;
		while (running){};
	}else{
		// Advertise VSPA streamer has started
		*v_RX_ext_dma_enabled = HOST_DMA_LINUX;
restart_rx:
		// Wait tx streaming to start
		printf("\n RX : Waiting _DDR_wr_address != 0xdeadbeef\n");
		fflush(stdout);
		ddr_wr_offset = 0;
		dmem_src_offset = 0;
		ddr_start_address =  *v_DDR_wr_base_address;
		//xfr_size=(*v_DDR_wr_CMP_enable?RX_DDR_STEP*RX_COMPRESS_RATIO_PCT/100:RX_DDR_STEP);
		while ((ddr_start_address == 0xdeadbeef) && running) {
			//microsleep();
			ddr_start_address =  *v_DDR_wr_base_address;
			};
		ddr_file_size =  *v_DDR_wr_size;
		local_RX_total_produced_size =  *v_RX_total_produced_size;
		local_RX_total_consumed_size =  *v_RX_total_consumed_size;
	    local_DDR_wr_load_start_bit_update = *v_DDR_wr_load_start_bit_update;

		// start feeding buffers
		while (running) {
			// wait produced data in dmem buffers
			do {
				loopcount++;
				// slow check stop event
				if(loopcount>500){
					loopcount=0;
					ddr_start_address=*v_DDR_wr_base_address;
					if(ddr_start_address==0xdeadbeef){
						goto restart_rx;
					}
				}
				if (local_DDR_wr_load_start_bit_update != 0) {
				// DMA perf test
					break;
				}
				// check DMA completion
				//PCI_DMA_READ_completion(NB_RCHAN);
			    // normal mode check for new transfer ready
				local_RX_total_produced_size =  *v_RX_total_produced_size;
				data_size = local_RX_total_produced_size - local_RX_total_consumed_size; // data QECed but not yet transfered
				if (data_size >= EXT_DMA_RX_DDR_STEP) {
					// ready to fetch new data
					local_RX_total_consumed_size += EXT_DMA_RX_DDR_STEP;
					break;
				}
				} while (running);

			// xfer one buffer
			ret = PCI_DMA_READ_transfer((uint32_t)(p_input_buffer) + dmem_src_offset, IQFLOOD_BUF_ADDR + (ddr_start_address-p_la9310_outbound_base) + ddr_wr_offset, EXT_DMA_RX_DDR_STEP,NB_RCHAN);
			if (ret) {
				host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]++;
			}

			// update pointers
			ddr_wr_offset += EXT_DMA_RX_DDR_STEP;
			if (ddr_wr_offset >= ddr_file_size) {
				ddr_wr_offset = 0;
			}
			dmem_src_offset += EXT_DMA_RX_DDR_STEP;
			if (dmem_src_offset >= (RX_NUM_BUF*RX_DDR_STEP)) {
				dmem_src_offset = 0;
			}
		}
	}
	
	*v_RX_ext_dma_enabled = HOST_DMA_OFF;

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}

#else
void *process_ant_rx_vspa_ext_dma(void *arg)
{
	perror("rx_vspa_ext_dma not supported (only 1T1R)");
	return 0;
}
#endif

