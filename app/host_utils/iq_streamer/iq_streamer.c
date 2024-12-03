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
#include <la9310_modinfo.h>
#include <la9310_host_if.h>

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

#include "vspa_dmem_proxy.h"
#include "iqmod_rx.h"
#include "iqmod_tx.h"
#include "iq_streamer.h"
#include "imx8-host.h"
#include "imx_edma_api.h"


void la9310_hexdump(const void *ptr, size_t sz);
void monitor_vspa_stats(void);
void print_vspa_trace(void);
void print_m7_trace(void);


volatile uint32_t running;

sem_t thread_sem;
void *process_ant_tx(void *arg);
void *process_ant_rx(void *arg);
void *process_ant_tx_vspa_ext_dma(void *arg);
void *process_ant_rx_vspa_ext_dma(void *arg);
void *process_ant_tx_streaming_app(void *arg);
void *process_ant_rx_streaming_app(void *arg);

uint32_t *v_iqflood_ddr_addr;
uint32_t *v_scratch_ddr_addr;
uint32_t *v_ocram_addr;
uint32_t *v_vspa_dmem_proxy; 
t_stats *host_stats;
t_tx_ch_host_proxy *tx_vspa_proxy_ro;
t_rx_ch_host_proxy *rx_vspa_proxy_ro;
uint32_t *v_tx_vspa_proxy_wo;
uint32_t *v_rx_vspa_proxy_wo;


uint32_t p_la9310_outbound_base;
uint32_t *BAR0_addr;
uint32_t *BAR1_addr;
uint32_t *BAR2_addr;
uint32_t *PCIE1_addr;

uint32_t ddr_file_addr = 0;
uint32_t ddr_file_size = 0;

uint32_t isVspaExtPciDMA = 1;
uint32_t isM7offload=0;
uint32_t app_fifo_addr = 0;
uint32_t app_fifo_size = 0;
uint32_t ddr_fifo_addr=0;
uint32_t ddr_fifo_size=0;
uint32_t ddr_rd_dma_xfr_size=TX_DDR_STEP;
uint32_t ddr_wr_dma_xfr_size=RX_DDR_STEP*RX_COMPRESS_RATIO_PCT/100;

void print_host_trace(void);

modinfo_t mi;

static int get_modem_info(int modem_id)
{
	int fd;
	int ret;
	char dev_name[32];
	sprintf(dev_name, "/dev/%s%d", LA9310_DEV_NAME_PREFIX, modem_id);

	fd = open(dev_name, O_RDWR);
	if (fd < 0) {
		printf("File %s open error\n", dev_name);
		return -1;
	}

	ret = ioctl(fd, IOCTL_LA93XX_MODINFO_GET, &mi);
	if (ret < 0) {
		printf("IOCTL_LA9310_MODINFO_GET failed.\n");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

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

	v_iqflood_ddr_addr = mmap(NULL, mi.iqflood.size, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, mi.iqflood.host_phy_addr);
	if (v_iqflood_ddr_addr == MAP_FAILED) {
		perror("Mapping v_iqflood_ddr_addr buffer failed\n");
		return -1;
	}

	v_scratch_ddr_addr = mmap(NULL,  mi.scratchbuf.size, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd,  mi.scratchbuf.host_phy_addr);
	if (v_scratch_ddr_addr == MAP_FAILED) {
		perror("Mapping v_scratch_ddr_addr buffer failed\n");
		return -1;
	}

	BAR0_addr = mmap(NULL, BAR0_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR0_ADDR);
	if (BAR0_addr == MAP_FAILED) {
		perror("Mapping BAR0_addr buffer failed\n");
		return -1;
	}

	BAR1_addr = mmap(NULL, BAR1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR1_ADDR);
	if (BAR1_addr == MAP_FAILED) {
		perror("Mapping BAR1_addr buffer failed\n");
		return -1;
	}

	BAR2_addr = mmap(NULL, BAR2_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR2_ADDR);
	if (BAR2_addr == MAP_FAILED) {
		perror("Mapping BAR2_addr buffer failed\n");
		return -1;
	}

	PCIE1_addr = mmap(NULL, IMX8MP_PCIE1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, IMX8MP_PCIE1_ADDR);
	if (PCIE1_addr == MAP_FAILED) {
		perror("Mapping PCIE1_addr buffer failed\n");
		return -1;
	}

	/* search outbound windows mapping iqflood */
	for (i = 0; i <= 3; i++) {
		*(volatile uint32_t *)((uint64_t)BAR0_addr + IATU_VIEWPORT_OFF) = i;
		if (*(volatile uint32_t *)((uint64_t)BAR0_addr + ATU_LWR_TARGET_ADDR_OFF_INBOUND_0) == mi.iqflood.host_phy_addr)
		{
			p_la9310_outbound_base =  *(volatile uint32_t *)((uint64_t)BAR0_addr + ATU_LWR_BASE_ADDR_OFF_OUTBOUND_0);
			break;
		}
	}
	if (i <= 3) {
		printf("la9310 pci outbound to ddr iqflood found : 0x%08x", p_la9310_outbound_base);
	} else {
		printf("la9310 pci outbound to iqflood (0x%08x) not found\n", (uint32_t)mi.iqflood.host_phy_addr);
		return -1;
	}

	if (pci_dma_mem_init((PCIE1_ADDR + DMA_OFFSET), PCIE1_SIZE, 0, 0, 0) < 0) {
		printf("Unable to MAP DMA REG area\n");
		return -1;
	}

	/* use last 256 bytes of iqflood as shared vspa dmem proxy , vspa will write mirrored dmem value to avoid PCI read from host */
	v_vspa_dmem_proxy = (uint32_t *)(v_iqflood_ddr_addr + (mi.iqflood.size - VSPA_DMEM_PROXY_SIZE)/4) ;
	host_stats=&(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy)->host_stats);
	rx_vspa_proxy_ro=&(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy)->rx_state_readonly[0]);
	tx_vspa_proxy_ro=&(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy)->tx_state_readonly);

	/* use dmem structure at hardcoded address to write host status/request */
	dccivac((uint32_t *)tx_vspa_proxy_ro);
	if (tx_vspa_proxy_ro->rx_num_chan == 1) {
		v_tx_vspa_proxy_wo = (uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00000000);
		v_rx_vspa_proxy_wo = (uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00000040);

	} else {
		v_tx_vspa_proxy_wo = (uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00004000);
		v_rx_vspa_proxy_wo = (uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00004040);
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
    fprintf(stderr, "\n|\t-t	Tx vspa ext dma");
    fprintf(stderr, "\n|\t-r	Rx vspa ext dma");
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
	int32_t c,i,ret;
	command_e command = 0;
	struct stat buffer;

	if(argc<=1){
		print_cmd_help();
		exit(1);
	}
		
   /* command line parser */
   while ((c = getopt(argc, argv, "hfrts:mdvxDMc")) != EOF)
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
	/*get modem info*/
        ret = get_modem_info(0);
        if(ret != 0) {
                perror("Fail to get modem_info \r\n");
		exit(EXIT_FAILURE);
        }
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
	
	if (command <= OP_TX_RX) {
		signal(SIGINT, terminate_process);
		signal(SIGTERM, terminate_process);
		signal(SIGUSR1, sigusr1_process);
	}

	if (command == OP_CLEAR_STATS) {
		for (i = 0; i < sizeof(t_stats)/4; i++) {
			*((uint32_t*)(host_stats)+i)=0;
		}
	}

	if (command == OP_TX_ONLY) {
		if(isVspaExtPciDMA)
			process_ant_tx_vspa_ext_dma(NULL);
		else 
			perror("not supported");
	}

	if (command == OP_RX_ONLY) {
		if(isVspaExtPciDMA)
			process_ant_rx_vspa_ext_dma(NULL);
		else 
			perror("not supported");
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

	pci_dma_mem_deinit(PCIE1_SIZE,0);
 
	return 0;
}


/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions 						*/
//VSPA_EXPORT(output_buffer)
//VSPA_EXPORT(TX_ext_dma_enabled)
//VSPA_EXPORT(TX_total_produced_size)
//VSPA_EXPORT(DDR_rd_load_start_bit_update)

uint32_t local_TX_total_produced_size=0; 
uint32_t local_TX_total_consumed_size=0;  
uint32_t local_DDR_rd_load_start_bit_update=0;

void *process_ant_tx_vspa_ext_dma(void *arg)
{
	int ret;
	uint32_t busy_size=0;
	uint32_t empty_size=0;
	uint32_t fifoWaterMark=0;
	uint32_t dmem_dst_offset=0;
	uint32_t ddr_start_address=0;
	uint32_t ddr_file_size=0;
	uint32_t ddr_rd_offset=0;
	uint32_t loopcount=0;
	uint32_t load_dma_test=0;

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
		dccivac((uint32_t*)(tx_vspa_proxy_ro));
		ddr_start_address=tx_vspa_proxy_ro->DDR_rd_base_address;
		while ((ddr_start_address==0xdeadbeef)&&running){
			//microsleep();
			dccivac((uint32_t*)(tx_vspa_proxy_ro));
			ddr_start_address=tx_vspa_proxy_ro->DDR_rd_base_address;
			};
		ddr_file_size=tx_vspa_proxy_ro->DDR_rd_size;

		// Check pointers are cleared
		local_TX_total_consumed_size=0;
		local_DDR_rd_load_start_bit_update=*v_DDR_rd_load_start_bit_update;
		local_TX_total_produced_size=0;
		// start feeding buffers
		while (running){
			// wait room in dmem buffers
			do {
				dccivac((uint32_t*)(tx_vspa_proxy_ro));
				ddr_start_address=tx_vspa_proxy_ro->DDR_rd_base_address;
				// stopped ?
				if(ddr_start_address==0xdeadbeef){
					goto restart_tx;
				}
				// DMA perf (don't wait for consumer)
				if(local_DDR_rd_load_start_bit_update!=0){
					empty_size=TX_NUM_BUF*TX_DDR_STEP - dmem_dst_offset;
					break;
				}
				// Normal mode check room for new transfer
				local_TX_total_consumed_size=tx_vspa_proxy_ro->la9310_fifo_consumed_size;
				busy_size = local_TX_total_produced_size - local_TX_total_consumed_size; 
				if(busy_size >(TX_NUM_BUF*TX_DDR_STEP)) {
					l1_trace(L1_TRACE_MSG_DMA_CFGERR, busy_size);
					busy_size = TX_NUM_BUF*TX_DDR_STEP;
				} 
				fifoWaterMark=TX_NUM_BUF*TX_DDR_STEP - dmem_dst_offset; 
				empty_size= (TX_NUM_BUF*TX_DDR_STEP) - busy_size; 
				if(empty_size >= TX_DDR_STEP){
					if(empty_size>fifoWaterMark)
						empty_size=fifoWaterMark;
					// ready for next transfer
					local_TX_total_produced_size += empty_size;
					host_stats->tx_stats[STAT_EXT_DMA_DDR_RD]+=empty_size/TX_DDR_STEP;
					break;
				}
			} while(1);

			// xfer one buffer
			ret= PCI_DMA_WRITE_transfer(mi.iqflood.host_phy_addr +(ddr_start_address-p_la9310_outbound_base)+ddr_rd_offset,(uint32_t)(p_output_buffer) + dmem_dst_offset,empty_size,NB_WCHAN);
			if(ret){
				host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]++;
				l1_trace(L1_TRACE_MSG_DMA_CFGERR, ret);
				l1_trace_disable=1;
			}

			// update pointers
			ddr_rd_offset+= empty_size;
			if(ddr_rd_offset>=ddr_file_size) {
				ddr_rd_offset=0;
			} 
			dmem_dst_offset+=empty_size;
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

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions						*/
//VSPA_EXPORT(input_qec_buffer)
//VSPA_EXPORT(RX_ext_dma_enabled)
//VSPA_EXPORT(RX_total_consumed_size)
//VSPA_EXPORT(DDR_wr_load_start_bit_update)

#ifdef IQMOD_RX_1R /* only 1T1R */
uint32_t local_RX_total_produced_size;
uint32_t local_RX_total_consumed_size;
uint32_t local_DDR_wr_load_start_bit_update;
void *process_ant_rx_vspa_ext_dma(void *arg)
{
	int ret;
	uint32_t data_size = 0;
	uint32_t fifoWaterMark=0;
	uint32_t dmem_src_offset = 0;
	uint32_t ddr_start_address = 0;
	uint32_t ddr_file_size = 0;
	uint32_t ddr_wr_offset = 0;
	//uint32_t xfr_size;
	uint32_t loopcount=0;
	
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
		dccivac((uint32_t*)(rx_vspa_proxy_ro));
		ddr_start_address=rx_vspa_proxy_ro->DDR_wr_base_address;
		while ((ddr_start_address == 0xdeadbeef) && running) {
			dccivac((uint32_t*)(rx_vspa_proxy_ro));
			ddr_start_address=rx_vspa_proxy_ro->DDR_wr_base_address;
			};
		ddr_file_size = rx_vspa_proxy_ro->DDR_wr_size;
		local_RX_total_produced_size =  0;
		local_RX_total_consumed_size =  0;

		PCI_DMA_READ_swreset();

	    local_DDR_wr_load_start_bit_update = *v_DDR_wr_load_start_bit_update;

		// start feeding buffers
		while (running) {
			// wait produced data in dmem buffers
			do {
				// check DMA completion
				PCI_DMA_READ_completion(0,NB_RCHAN);

				// check new transfer
				dccivac((uint32_t*)(rx_vspa_proxy_ro));
				ddr_start_address=rx_vspa_proxy_ro->DDR_wr_base_address;
				// stopped ?
				if(ddr_start_address==0xdeadbeef){
					goto restart_rx;
				}
				// DMA perf test
				if (local_DDR_wr_load_start_bit_update != 0) {
					data_size=RX_NUM_QEC_BUF*RX_DDR_STEP-dmem_src_offset;
					break;
				}
			    // Normal mode check for new transfer ready
				local_RX_total_produced_size = rx_vspa_proxy_ro->la9310_fifo_produced_size;
				data_size = local_RX_total_produced_size - local_RX_total_consumed_size; // data QECed but not yet transfered
				fifoWaterMark=RX_NUM_QEC_BUF*RX_DDR_STEP-dmem_src_offset; 
				if (data_size >= RX_DDR_STEP /*EXT_DMA_RX_DDR_STEP*/) {
					if(data_size>fifoWaterMark)
						data_size=fifoWaterMark;
					// ready to fetch new data
					local_RX_total_consumed_size += data_size;
					host_stats->rx_stats[0][STAT_EXT_DMA_DDR_WR]+=data_size/RX_DDR_STEP;
					break;
				}
				} while (running);

			// xfer 1 or more buffers
			ret = PCI_DMA_READ_transfer((uint32_t)(p_input_qec_buffer) + dmem_src_offset, mi.iqflood.host_phy_addr + (ddr_start_address-p_la9310_outbound_base) + ddr_wr_offset, data_size,0,NB_RCHAN,0);
			if (ret) {
				host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]++;
				l1_trace(L1_TRACE_MSG_DMA_AXIQ_TX_XFER_ERROR, ret);
				l1_trace_disable=1;
			}

			// update pointers
			ddr_wr_offset += data_size;
			if (ddr_wr_offset >= ddr_file_size) {
				ddr_wr_offset = 0;
			}
			dmem_src_offset += data_size	;
			if (dmem_src_offset >= (RX_NUM_QEC_BUF*RX_DDR_STEP)) {
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

//VSPA_EXPORT(input_dec_buffer)
t_local_rx_ch_context local_rx_ch_context[RX_NUM_CHAN];
uint32_t local_DDR_wr_load_start_bit_update;

uint32_t data_size = 0,ddr_start_address=0;
uint32_t readyForDMA = 0;
uint32_t dma_chan = 0;

void *process_ant_rx_vspa_ext_dma(void *arg)
{
	int ret,i;
	//uint32_t data_size = 0,ddr_start_address=0;
	
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
		dccivac((uint32_t*)(rx_vspa_proxy_ro));
		ddr_start_address=rx_vspa_proxy_ro[RX_NUM_CHAN-1].DDR_wr_base_address;
		while ((ddr_start_address == 0xdeadbeef) && running) {
			dccivac((uint32_t*)(rx_vspa_proxy_ro));
			ddr_start_address=rx_vspa_proxy_ro[RX_NUM_CHAN-1].DDR_wr_base_address;
			};

		for(i=0;i<RX_NUM_CHAN;i++){
			local_rx_ch_context[i].RX_total_produced_size=0; 
			local_rx_ch_context[i].RX_total_consumed_size=0; 
			local_rx_ch_context[i].ddr_wr_offset=0;
			local_rx_ch_context[i].dmem_src_offset=0;
			local_rx_ch_context[i].DDR_wr_base_address=mi.iqflood.host_phy_addr + (rx_vspa_proxy_ro[i].DDR_wr_base_address-p_la9310_outbound_base); 
			local_rx_ch_context[i].dmem_rd_base_address=(uint32_t)(p_input_dec_buffer) + i*RX_NUM_DEC_BUF*(RX_DMA_TXR_STEP/RX_DECIM) ;
			local_rx_ch_context[i].DDR_wr_size=rx_vspa_proxy_ro[i].DDR_wr_size;
		}



		PCI_DMA_READ_swreset();

	    local_DDR_wr_load_start_bit_update = *v_DDR_wr_load_start_bit_update;

		// start feeding buffers
		while (running) {
			// check DMA completion
			for(i=0;i<DMA_READ_MAX_NB_CHAN;i++){
				// check DMA completion
				PCI_DMA_READ_completion(i,1);
			}
			// check new transfer
			for(i=0;i<RX_NUM_CHAN;i++){
//				uint32_t readyForDMA = 0;
//				uint32_t dma_chan = 0;

				dccivac((uint32_t*)(rx_vspa_proxy_ro));

				// stopped ?
				ddr_start_address=rx_vspa_proxy_ro[RX_NUM_CHAN-1].DDR_wr_base_address;
				if(ddr_start_address==0xdeadbeef){
					goto restart_rx;
				}

				readyForDMA=0;
				// DMA perf test
				if (local_DDR_wr_load_start_bit_update != 0) {
					readyForDMA=1;
				}

				// Normal mode check for new transfer ready
				local_rx_ch_context[i].RX_total_produced_size = rx_vspa_proxy_ro[i].la9310_fifo_produced_size;
				data_size = local_rx_ch_context[i].RX_total_produced_size - local_rx_ch_context[i].RX_total_consumed_size; // data QECed but not yet transfered
				if (data_size >= EXT_DMA_RX_DDR_STEP) {
					// ready to fetch new data
					readyForDMA=1;
				}

				// check DMA is available
				dma_chan=PCI_DMA_READ_available();
				if( readyForDMA && (dma_chan!=-1) ) {
					local_rx_ch_context[i].RX_total_consumed_size += EXT_DMA_RX_DDR_STEP;
					// xfer 1 or more buffers
					ret = PCI_DMA_READ_transfer( local_rx_ch_context[i].dmem_rd_base_address + local_rx_ch_context[i].dmem_src_offset    
												, local_rx_ch_context[i].DDR_wr_base_address + local_rx_ch_context[i].ddr_wr_offset
												, EXT_DMA_RX_DDR_STEP,dma_chan,1,i);
					if (ret) {
						host_stats->gbl_stats[ERROR_DMA_XFER_ERROR]++;
					}

					// update pointers
					local_rx_ch_context[i].ddr_wr_offset += EXT_DMA_RX_DDR_STEP;
					if (local_rx_ch_context[i].ddr_wr_offset >= local_rx_ch_context[i].DDR_wr_size) {
						local_rx_ch_context[i].ddr_wr_offset = 0;
					}
					local_rx_ch_context[i].dmem_src_offset += EXT_DMA_RX_DDR_STEP;
					if (local_rx_ch_context[i].dmem_src_offset >= (RX_NUM_DEC_BUF*RX_DDR_STEP)) {
						local_rx_ch_context[i].dmem_src_offset = 0;
					}
				}
			}
		}
	}
	
	*v_RX_ext_dma_enabled = HOST_DMA_OFF;

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}
#endif

