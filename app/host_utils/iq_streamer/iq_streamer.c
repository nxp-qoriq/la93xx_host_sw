/* Copyright 2022-2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
#include "vspa_exported_symbols.h"
#include "iq_streamer.h"
#include "pci_imx8mp.h"
#include "stats.h"
#include "la9310_regs.h"

#define microsleep() {volatile uint64_t i; for(i=0;i<1000;i++){};}

volatile uint32_t running=0;

sem_t thread_sem;
void *process_ant_tx(void *arg);
void *process_ant_rx(void *arg);
pthread_t ant_thread[NUM_ANT];
int ant_thread_arg[] = {0, 1, 2, 3, 4, 5}; 
int num_ant_enabled;

uint32_t *_VSPA_A011455;
uint32_t *iqflood_ddr_addr;
uint32_t *BAR0_addr;
uint32_t *BAR1_addr;
uint32_t *BAR2_addr;
uint32_t *PCIE1_addr;

volatile uint32_t *_VSPA_cyc_count_lsb;
volatile uint32_t *_VSPA_cyc_count_msb;

int map_physical_regions(void)
{
	int devmem_fd;
	/*
	* map memory regions
	*/
   
	devmem_fd = open("/dev/mem", O_RDWR);
	if (-1 == devmem_fd) {
		perror("/dev/mem open failed");
		return -1;
	}

	iqflood_ddr_addr = mmap(NULL, IQFLOOD_BUF_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, IQFLOOD_BUF_ADDR);
	if (MAP_FAILED == iqflood_ddr_addr) {
		perror("Mapping iqflood_ddr_addr buffer failed\n");
		return -1;
	}
//	printf("\n map iqflood_ddr :");
//	la9310_hexdump(iqflood_ddr_addr,64);

	BAR0_addr = mmap(NULL, BAR0_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR0_ADDR);
	if (MAP_FAILED == BAR0_addr) {
		perror("Mapping BAR0_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR0_addr :");
//	la9310_hexdump(BAR0_addr,64);

	BAR1_addr = mmap(NULL, BAR1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR1_ADDR);
	if (MAP_FAILED == BAR1_addr) {
		perror("Mapping BAR1_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR1_addr :");
//	la9310_hexdump(BAR1_addr,64);

	BAR2_addr = mmap(NULL, BAR2_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR2_ADDR);
	if (MAP_FAILED == BAR2_addr) {
		perror("Mapping BAR2_addr buffer failed\n");
		return -1;
	}
//	printf("\n map BAR2_addr :");
//	la9310_hexdump(BAR2_addr,64);

	PCIE1_addr = mmap(NULL, IMX8MP_PCIE1_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, IMX8MP_PCIE1_ADDR);
	if (MAP_FAILED == PCIE1_addr) {
		perror("Mapping PCIE1_addr buffer failed\n");
		return -1;
	}
//	printf("\n map PCIE1_addr :");
//	la9310_hexdump(PCIE1_addr,64);

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
    running=0;
	*v_TX_iqstreamer_enabled=0;
	*v_RX_iqstreamer_enabled=0;
	print_history();
}

void sigusr1_process(int sig)
{
    fflush(stdout);
	print_history();

}

void print_cmd_help(void)
{
    fprintf(stderr, "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    fprintf(stderr, "\n|    iq_streamer");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| ./iq_streamer -s "); 
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| Trace");
    fprintf(stderr, "\n|\t-h 	help");
    fprintf(stderr, "\n|\t-t 	Tx streaming only");
    fprintf(stderr, "\n|\t-r 	Rx streaming only");
    fprintf(stderr, "\n|\t-a 	TX and Rx streaming");
    fprintf(stderr, "\n|\t-s 	get stats");
    fprintf(stderr, "\n|\t-m 	monitor stats");
    fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

int main(int argc, char *argv[])
{
	int ret;
	cpu_set_t cpuset;
    int32_t c;
	command_e command=0;
	struct stat buffer;

	if(stat("/sys/shiva/shiva_status", &buffer) != 0){
		perror("la9310 shiva driver not started");
		exit(EXIT_FAILURE);
	}


   /* command line parser */
   while (( c = getopt(argc,argv, "hrtsmd"))!=EOF)		
   {
		switch (c) {
			case 'h': // help
				print_cmd_help();
				exit(1);
			break;
			case 'a':
				command=OP_TX_RX;
			break;
			case 't':
				command=OP_TX_ONLY;
			break;
			case 'r':
				command=OP_RX_ONLY;
			break;
			case 's':
				command=OP_STATS;
			break;
			case 'm':
				command=OP_MONITOR;
			break;
			case 'd':
				command=OP_DUMP_TRACE;
			break;
			default:
				print_cmd_help();
				exit(1);
			break;

		}
	}

	running=1;

	/*
	* map memory regions
	*/
	map_physical_regions();

	/*
	 *   start stream Tx IQ data thread
	 */

	if(command<=OP_TX_RX){
		signal(SIGINT, terminate_process);
		signal(SIGTERM, terminate_process);
		signal(SIGUSR1, sigusr1_process);
	}

	if(command==OP_TX_RX){

		if (sem_init(&thread_sem, 0, 0)) {
			perror("sem_init failed:");
			exit(EXIT_FAILURE);
		}

		/* Launch antenna thread */
		ret = pthread_create(&ant_thread[0], NULL, process_ant_tx, &ant_thread_arg[0]);
		if (ret) {
			perror("Antenna thread creation failed:");
			exit(EXIT_FAILURE);
		}

		/* Migrate thread to a isolated cpu core */
		CPU_ZERO(&cpuset);
		CPU_SET(3, &cpuset);
		ret = pthread_setaffinity_np(ant_thread[0], sizeof(cpuset), &cpuset);
		if (ret) {
			perror("pthread_setaffinity_np failed");
			exit(EXIT_FAILURE);
		}
		/* Launch antenna thread */
		ret = pthread_create(&ant_thread[1], NULL, process_ant_rx, &ant_thread_arg[1]);
		if (ret) {
			perror("Antenna thread creation failed:");
			exit(EXIT_FAILURE);
		}

		/* Migrate thread to a isolated cpu core */
		CPU_ZERO(&cpuset);
		CPU_SET(2, &cpuset);
		ret = pthread_setaffinity_np(ant_thread[1], sizeof(cpuset), &cpuset);
		if (ret) {
			perror("pthread_setaffinity_np failed");
			exit(EXIT_FAILURE);
		}

		num_ant_enabled=2;
		while (running)
		{
		  sleep(1); /* sleep 1s */	
		}

		for (int count = 0; count < num_ant_enabled; count++) {
			if (sem_wait(&thread_sem))
				perror("sem_wait failed:");
		}
	}
	
	if(command==OP_TX_ONLY){
		num_ant_enabled=1;
		process_ant_tx(NULL);
	}

	if(command==OP_RX_ONLY){
		num_ant_enabled=1;
		process_ant_rx(NULL);
	}

	if(command==OP_STATS){
		print_vspa_stats();
	}

	if(command==OP_MONITOR){
		monitor_vspa_stats();
	}

	if(command==OP_DUMP_TRACE){
		print_vspa_trace();
	}
	
	return 0;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions 						*/
//VSPA_EXPORT(DDR_base_address)
//VSPA_EXPORT(DDR_rd_size)
//VSPA_EXPORT(TX_total_produced_size)
//VSPA_EXPORT(TX_total_consumed_size)
//VSPA_EXPORT(output_buffer)
//VSPA_EXPORT(TX_iqstreamer_enabled)

uint32_t local_TX_total_produced_size=0;
uint32_t local_TX_total_consumed_size=0;

void *process_ant_tx(void *arg)
{
	int ret;
	uint32_t busy_size=0;
	uint32_t empty_size=0;
	uint32_t dmem_dst_offset=0;
	uint32_t ddr_start_address=0;
	uint32_t ddr_file_size=0;
	uint32_t ddr_rd_offset=0;
	uint32_t ant=1;

	_VSPA_cyc_count_lsb= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_LSB);
	_VSPA_cyc_count_msb= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_MSB);

	// !! this workaround is needed otherwise VSPA gets stuck ( dmac_is_available()/dmac_is_complete() doesn't work)  
	// !! unless issue "ccs::config_chain {la9310 dap}" on ccs
	_VSPA_A011455= (uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + 0x24);
	*_VSPA_A011455|=0x10;
	
	// Advertise VSPA streamer has started
	*v_TX_iqstreamer_enabled=1;

restart_tx:
	// Wait tx streaming to start
	printf("\n TX : Waiting _DDR_base_address != 0xdeadbeef \n");
	fflush(stdout);
	dmem_dst_offset=0;
	ddr_rd_offset=0;
	ddr_start_address=*v_DDR_base_address;
	while ((ddr_start_address==0xdeadbeef)&&running){
		//microsleep();
		ddr_start_address=*v_DDR_base_address;
		};
	ddr_file_size=*v_DDR_rd_size;

	// Check pointers are cleared
	local_TX_total_consumed_size=*v_TX_total_consumed_size;
	if(local_TX_total_consumed_size&&running){
		printf("\n Error TX_total_consumed_size=0x%08x not zero !",local_TX_total_consumed_size);
		};
	local_TX_total_produced_size=*v_TX_total_produced_size;
	if(local_TX_total_produced_size&&running){
		printf("\n Error TX_total_consumed_size=0x%08x not zero !",local_TX_total_produced_size);
		};
	
	// start feeding buffers
	while (running){
		GET_VSPA_CYCLE_COUNT(cycle_count_idle_start,ant,cycle_count_index[ant]);
		// wait room in dmem buffers
		do {
			local_TX_total_consumed_size=*v_TX_total_consumed_size;
			busy_size= local_TX_total_produced_size - local_TX_total_consumed_size;
			empty_size= (TX_NUM_BUF*DDR_STEP) - busy_size; 
			if(empty_size < DDR_STEP){ 
				//microsleep(); //polling dmem may be a pb ?
			if((ddr_start_address==0xdeadbeef)&&running)
				goto restart_tx;
			}
			else{
				break;
			}
			} while(running);

		// xfer one buffer
		GET_VSPA_CYCLE_COUNT(cycle_count_idle_done,ant,cycle_count_index[ant]);
#if 1
		//ret= PCI_DMA_WRITE_transfer(0x96400000 + (ddr_start_address&0x0FFFFFFF)+ddr_rd_offset,(uint32_t)(p_l1_trace_data),DDR_STEP,ant);
		//ret=PCI_DMA_WRITE_transfer(0x96400000 + (ddr_start_address&0x0FFFFFFF)+ddr_rd_offset,0x1c000000,DDR_STEP);
		ret= PCI_DMA_WRITE_transfer(0x96400000 + (ddr_start_address&0x0FFFFFFF)+ddr_rd_offset,(uint32_t)(p_output_buffer) + dmem_dst_offset,DDR_STEP,ant);
		if(ret){
			printf("\n!! DMA not ready !!\n");
			break;
		}
#endif

		// update pointers
		ddr_rd_offset+= DDR_STEP;
		if(ddr_rd_offset>=ddr_file_size) {
			ddr_rd_offset=0;
		} 
		dmem_dst_offset+=DDR_STEP;
		if(dmem_dst_offset>=(TX_NUM_BUF*DDR_STEP)) {
			dmem_dst_offset=0;
		}
		local_TX_total_produced_size+=DDR_STEP;
		*v_TX_total_produced_size=local_TX_total_produced_size;
		
		// capture only long cycles ( should be 0x1000 cycle at 122.88MSPS )
		if(!stats_locked){
			cycle_count_duration[ant][cycle_count_index[ant]]=cycle_count_xfer_done[ant][cycle_count_index[ant]]-cycle_count_idle_start[ant][cycle_count_index[ant]];
			if(cycle_count_duration[ant][cycle_count_index[ant]]> 0x3000 ) {
				cycle_count_max_duration[ant][cycle_count_max_index[ant]]=cycle_count_duration[ant][cycle_count_index[ant]];
				cycle_count_max_date[ant][cycle_count_max_index[ant]]=cycle_count_idle_start[ant][cycle_count_index[ant]];
				cycle_count_max_index[ant]++;
				if(cycle_count_max_index[ant]==MAX_CYC_CNT) {
					cycle_count_max_index[ant]=0;
				}
			}
			cycle_count_index[ant]++;
			if(cycle_count_index[ant]==MAX_CYC_CNT) {
				cycle_count_index[ant]=0;
			}
		}
		
	}

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions 						*/
//VSPA_EXPORT(DDR_wr_address)
//VSPA_EXPORT(DDR_wr_size)
//VSPA_EXPORT(RX_total_produced_size)
//VSPA_EXPORT(RX_total_consumed_size)
//VSPA_EXPORT(input_buffer)
//VSPA_EXPORT(RX_iqstreamer_enabled)

uint32_t local_RX_total_produced_size=0;
uint32_t local_RX_total_consumed_size=0;

void *process_ant_rx(void *arg)
{
	int ret;
	uint32_t data_size=0;
	uint32_t dmem_src_offset=0;
	uint32_t ddr_start_address=0;
	uint32_t ddr_file_size=0;
	uint32_t ddr_wr_offset=0;
	uint32_t ant=0;

	_VSPA_cyc_count_lsb= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_LSB);
	_VSPA_cyc_count_msb= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_MSB);

	// !! this workaround is needed otherwise VSPA gets stuck ( dmac_is_available()/dmac_is_complete() doesn't work)  
	// !! unless issue "ccs::config_chain {la9310 dap}" on ccs
	_VSPA_A011455= (uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + 0x24);
	*_VSPA_A011455|=0x10;

	// Advertise VSPA streamer has started
	*v_RX_iqstreamer_enabled=1;

restart_rx:
	// Wait tx streaming to start
	printf("\n RX : Waiting _DDR_wr_address != 0xdeadbeef \n");
	fflush(stdout);
	ddr_wr_offset=0;
	dmem_src_offset=0;
	ddr_start_address=*v_DDR_wr_address;
	while ((ddr_start_address==0xdeadbeef)&&running){
		//microsleep();
		ddr_start_address=*v_DDR_wr_address;
		};
	ddr_file_size=*v_DDR_wr_size;

	// Check pointers are cleared
	local_RX_total_consumed_size=*v_RX_total_consumed_size;
	if(local_RX_total_consumed_size&&running){
		printf("\n Error RX_total_consumed_size=0x%08x not zero !",local_RX_total_consumed_size);
		};
	
	// start feeding buffers
	while (running){
		
		GET_VSPA_CYCLE_COUNT(cycle_count_idle_start,ant,cycle_count_index[ant]);

		// wait produced data in dmem buffers
		do {
			local_RX_total_produced_size=*v_RX_total_produced_size;
			data_size= local_RX_total_produced_size - local_RX_total_consumed_size;
			if(data_size < DDR_STEP){ 
				//microsleep(); //polling dmem may be a pb ?
			}
			else{
				break;
			}
			if((ddr_start_address==0xdeadbeef)&&running)
				goto restart_rx;
			} while(running);

		// xfer one buffer
		GET_VSPA_CYCLE_COUNT(cycle_count_idle_done,ant,cycle_count_index[ant]);
		ret= PCI_DMA_READ_transfer((uint32_t)(p_input_buffer) + dmem_src_offset,0x96400000 + (ddr_start_address&0x0FFFFFFF)+ddr_wr_offset,DDR_STEP,ant);
		if(ret){
			printf("\n!! DMA not ready !!\n");
			break;
		}

		// update pointers
		ddr_wr_offset+= DDR_STEP;
		if(ddr_wr_offset>=ddr_file_size) {
			ddr_wr_offset=0;
		} 
		dmem_src_offset+=DDR_STEP;
		if(dmem_src_offset>=(RX_NUM_BUF*DDR_STEP)) {
			dmem_src_offset=0;
		}
		local_RX_total_consumed_size+=DDR_STEP;
		*v_RX_total_consumed_size=local_RX_total_consumed_size;
		
		// capture only long cycles ( should be 0x1000 cycle at 122.88MSPS )
		if(!stats_locked){
			cycle_count_duration[ant][cycle_count_index[ant]]=cycle_count_xfer_done[ant][cycle_count_index[ant]]-cycle_count_idle_start[ant][cycle_count_index[ant]];
			if(cycle_count_duration[ant][cycle_count_index[ant]]> 0x3000 ) {
				cycle_count_max_duration[ant][cycle_count_max_index[ant]]=cycle_count_duration[ant][cycle_count_index[ant]];
				cycle_count_max_date[ant][cycle_count_max_index[ant]]=cycle_count_idle_start[ant][cycle_count_index[ant]];
				cycle_count_max_index[ant]++;
				if(cycle_count_max_index[ant]==MAX_CYC_CNT) {
					cycle_count_max_index[ant]=0;
				}
			}
			cycle_count_index[ant]++;
			if(cycle_count_index[ant]==MAX_CYC_CNT) {
				cycle_count_index[ant]=0;
			}
		}
	}

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}
