/* Copyright 2022-2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
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
#include "vspa_exported_symbols.h"
#include "iq_streamer.h"
#ifndef IMX8DXL
#include "pci_imx8mp.h"
#else
#include "pci_imx8dxl.h"
#endif
#include "stats.h"
#include "la9310_regs.h"
#include "l1-trace.h"


#define microsleep() {volatile uint64_t i; for (i = 0; i < 1000; i++) {}; }

volatile uint32_t running;

sem_t thread_sem;
void *process_ant_tx(void *arg);
void *process_ant_rx(void *arg);
pthread_t ant_thread[NUM_ANT];
int ant_thread_arg[] = {0, 1, 2, 3, 4, 5};
int num_ant_enabled;

uint32_t *_VSPA_A011455;
uint32_t *v_iqflood_ddr_addr;
uint32_t *v_scratch_ddr_addr;

uint32_t p_la9310_outbound_base;
uint32_t *BAR0_addr;
uint32_t *BAR1_addr;
uint32_t *BAR2_addr;
uint32_t *PCIE1_addr;

uint32_t ddr_file_addr;
uint32_t ddr_file_size;

volatile uint32_t *_VSPA_cyc_count_lsb;
volatile uint32_t *_VSPA_cyc_count_msb;

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
    fprintf(stderr, "\n|\t-t	Tx streaming only");
    fprintf(stderr, "\n|\t-r	Rx streaming only");
    fprintf(stderr, "\n|\t-a	ddr buff address");
    fprintf(stderr, "\n|\t-s	ddr buff size");
    fprintf(stderr, "\n|\t-m	monitor stats");
    fprintf(stderr, "\n|\t-d	dump vspa trace");
    fprintf(stderr, "\n|\t-v	version");
    fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

int main(int argc, char *argv[])
{
	int ret;
	cpu_set_t cpuset;
    int32_t c;
	command_e command = 0;
	struct stat buffer;

   /* command line parser */
   while ((c = getopt(argc, argv, "hfrts:a:mdvx")) != EOF)
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
			break;
			case 's':
				ddr_file_size = strtoull(argv[optind-1], 0, 0);
			break;
			case 'm':
				command = OP_MONITOR;
			break;
			case 'd':
				command = OP_DUMP_TRACE;
			break;
			case 'x':
				command = OP_DMA_PERF;
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

	if (command <= OP_TX_RX) {
		signal(SIGINT, terminate_process);
		signal(SIGTERM, terminate_process);
		signal(SIGUSR1, sigusr1_process);
	}

	if (command == OP_TX_RX) {

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

		num_ant_enabled = 2;
		while (running)
		{
		  sleep(1); /* sleep 1s */
		}

		for (int count = 0; count < num_ant_enabled; count++) {
			if (sem_wait(&thread_sem))
				perror("sem_wait failed:");
		}
	}

	if (command == OP_TX_ONLY) {
		num_ant_enabled = 1;
		process_ant_tx(NULL);
	}

	if (command == OP_RX_ONLY) {
		num_ant_enabled = 1;
		process_ant_rx(NULL);
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

	if (command == OP_DMA_PERF) {
		dma_perf_test();
	}

	return 0;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions						*/
//VSPA_EXPORT(DDR_rd_base_address)
//VSPA_EXPORT(DDR_rd_size)
//VSPA_EXPORT(TX_total_consumed_size)
//VSPA_EXPORT(DDR_rd_load_start_bit_update)

uint32_t local_TX_total_produced_size;
uint32_t local_TX_total_consumed_size;

void *process_ant_tx(void *arg)
{
	int ret;
	uint32_t busy_size = 0;
	uint32_t empty_size = 0;
	uint32_t tcm_fifo_size = 0;
	uint32_t ep_tcm_fifo_address = 0;
	uint32_t tcm_fifo_addr = 0;
	uint32_t tcm_dst_offset = 0;
	uint32_t ddr_rd_offset = 0;
	uint32_t ant = 1;
	uint32_t txcount = 0;

	_VSPA_cyc_count_lsb = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_LSB);
	_VSPA_cyc_count_msb = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_MSB);

	// !! this workaround is needed otherwise VSPA gets stuck ( dmac_is_available()/dmac_is_complete() doesn't work)
	// !! unless issue "ccs::config_chain {la9310 dap}" on ccs
	_VSPA_A011455 = (uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + 0x24);
	*_VSPA_A011455 |= 0x10;

	// Advertise VSPA streamer has started
	//*v_TX_ext_dma_enabled=1;

restart_tx:
	// Wait tx streaming to start
	printf("\n TX : Waiting _DDR_base_address != 0xdeadbeef\n");
	fflush(stdout);
	tcm_dst_offset = 0;
	ddr_rd_offset = 0;

#if 0
	while ((ep_tcm_fifo_address == 0xdeadbeef) && running) {
		//microsleep();
		ep_tcm_fifo_address =  *v_DDR_rd_base_address;
		};
	if (ep_tcm_fifo_address != TCM_TX_FIFO) {
		printf("\n VSPA tx started from DDR , exit TCM iq_streamer\n");
		return 0;
	}
	if (ep_tcm_fifo_address != TCM_TX_FIFO) {
		printf("\n VSPA tx started from DDR , exit TCM iq_streamer\n");
		return 0;
	}
	tcm_fifo_size =  *v_DDR_rd_size;
	if (tcm_fifo_size < 0x4000) {
		printf("\n tcm_fifo_size 0x%08x should > 16KB\n", tcm_fifo_size);
		return 0;
	}
	if ((tcm_fifo_size%TX_DDR_STEP) != 0) {
		printf("\n tcm_fifo_size 0x%08x should be multiple of 0x%08x\n", tcm_fifo_size, TX_DDR_STEP);
		return 0;
	}
#endif

	// hardcoded 32KB TCM TX FIFO
	ep_tcm_fifo_address = TCM_TX_FIFO;
	tcm_fifo_size = 0x8000;

	if (ddr_file_addr == 0) {
		printf("\n invalid ddr_file_addr\n");
		return 0;
	}
	if (ddr_file_size == 0) {
		printf("\n invalid ddr_file_size\n");
		return 0;
	}

	tcm_fifo_addr = (uint64_t)BAR2_ADDR + ep_tcm_fifo_address - TCM_START;
	txcount =  *(v_g_stats+STAT_EXT_DMA_DDR_RD);

	// Check pointers are cleared
	local_TX_total_consumed_size =  *v_TX_total_consumed_size;
	if (local_TX_total_consumed_size) {
		printf("\n Error TX_total_consumed_size=0x%08x not zero !", local_TX_total_consumed_size);
		return 0;
	};

	// start feeding buffers
	while (running) {
		// wait room in TCM TX FIFO buffers
		do {
			if (*v_DDR_rd_load_start_bit_update != 0) {
			// DMA perf test
				break;
			}
			if (local_TX_total_produced_size > tcm_fifo_size) {
				ep_tcm_fifo_address =  *v_DDR_rd_base_address;
				if (ep_tcm_fifo_address == 0xdeadbeef) {
					// stopped ?
					ep_tcm_fifo_address =  *v_DDR_rd_base_address;
					goto restart_tx;
				}
			}
			local_TX_total_consumed_size =  *v_TX_total_consumed_size;
			busy_size = local_TX_total_produced_size - local_TX_total_consumed_size;
			empty_size = tcm_fifo_size - busy_size; // fifo size - data in dmem
			if (empty_size >= TX_DDR_STEP) {
				break;
			}
			if ((empty_size == tcm_fifo_size) && !local_TX_total_produced_size) {
				*(v_g_stats+ERROR_EXT_DMA_DDR_RD_UNDERRUN) += 1;
			}
			//microsleep(); //polling dmem may be a pb ?
		} while (running);

		ret = PCI_DMA_WRITE_transfer(ddr_file_addr+ddr_rd_offset, tcm_fifo_addr + tcm_dst_offset, TX_DDR_STEP, ant);
		txcount += TX_DDR_STEP/DDR_STEP;
		*(v_g_stats+STAT_EXT_DMA_DDR_RD) = txcount;
		if (ret) {
			printf("\n!! DMA not ready !!\n");
			break;
		}

		ddr_rd_offset += TX_DDR_STEP;
		if (ddr_rd_offset >= ddr_file_size) {
			ddr_rd_offset = 0;
		}
		tcm_dst_offset += TX_DDR_STEP;
		if (tcm_dst_offset >= tcm_fifo_size) {
			tcm_dst_offset = 0;
		}
		local_TX_total_produced_size += TX_DDR_STEP;
	}

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}


/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
/* !! buffer should not cross vcpu/ippu regions						*/
	//VSPA_EXPORT(DDR_wr_base_address)
	//VSPA_EXPORT(DDR_wr_size)
	//VSPA_EXPORT(RX_total_produced_size)
	//VSPA_EXPORT(RX_total_consumed_size)
//VSPA_EXPORT(input_buffer)
//VSPA_EXPORT(RX_ext_dma_enabled)
//VSPA_EXPORT(DDR_wr_load_start_bit_update)

uint32_t local_RX_total_produced_size;
uint32_t local_RX_total_consumed_size;

void *process_ant_rx(void *arg)
{
	int ret;
	uint32_t data_size = 0;
	uint32_t dmem_src_offset = 0;
	uint32_t ddr_start_address = 0;
	uint32_t ddr_file_size = 0;
	uint32_t ddr_wr_offset = 0;
	uint32_t ant = 0;
	uint32_t rxcount = 0;

	_VSPA_cyc_count_lsb = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_LSB);
	_VSPA_cyc_count_msb = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + CYC_COUNTER_MSB);

	// !! this workaround is needed otherwise VSPA gets stuck ( dmac_is_available()/dmac_is_complete() doesn't work)
	// !! unless issue "ccs::config_chain {la9310 dap}" on ccs
	_VSPA_A011455 = (uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + 0x24);
	*_VSPA_A011455 |= 0x10;

	// Advertise VSPA streamer has started
	*v_RX_ext_dma_enabled = 1;

restart_rx:
	// Wait tx streaming to start
	printf("\n RX : Waiting _DDR_wr_address != 0xdeadbeef\n");
	fflush(stdout);
	ddr_wr_offset = 0;
	dmem_src_offset = 0;
	ddr_start_address =  *v_DDR_wr_base_address;
	while ((ddr_start_address == 0xdeadbeef) && running) {
		//microsleep();
		ddr_start_address =  *v_DDR_wr_base_address;
		};
	ddr_file_size =  *v_DDR_wr_size;
	rxcount =  *(v_g_stats+STAT_EXT_DMA_DDR_WR);

	// Check pointers are cleared
	local_RX_total_consumed_size =  *v_RX_total_consumed_size;
	if (local_RX_total_consumed_size && running) {
		printf("\n Error RX_total_consumed_size=0x%08x not zero !", local_RX_total_consumed_size);
		};

	// start feeding buffers
	while (running) {
		// wait produced data in dmem buffers
		do {
			if (*v_DDR_wr_load_start_bit_update != 0) {
			// DMA perf test
				break;
			}
			ddr_start_address =  *v_DDR_wr_base_address;
			if (ddr_start_address == 0xdeadbeef)
			// stopped ?
				goto restart_rx;
			local_RX_total_produced_size =  *v_RX_total_produced_size;
			data_size = local_RX_total_produced_size - local_RX_total_consumed_size; // data QECed but not yet transfered
			if (data_size >= DDR_STEP) {
			// check
				break;
			}
			//microsleep(); //polling dmem may be a pb ?
			} while (running);

		// xfer one buffer
		ret = PCI_DMA_READ_transfer((uint32_t)(p_input_buffer) + dmem_src_offset, IQFLOOD_BUF_ADDR + (ddr_start_address-p_la9310_outbound_base) + ddr_wr_offset, DDR_STEP, ant);
		rxcount++;
		*(v_g_stats+STAT_EXT_DMA_DDR_WR) = rxcount;
		if (ret) {
			printf("\n!! DMA not ready !!\n");
			break;
		}

		// update pointers
		ddr_wr_offset += DDR_STEP;
		if (ddr_wr_offset >= ddr_file_size) {
			ddr_wr_offset = 0;
		}
		dmem_src_offset += DDR_STEP;
		if (dmem_src_offset >= (RX_NUM_BUF*DDR_STEP)) {
			dmem_src_offset = 0;
		}
		local_RX_total_consumed_size += DDR_STEP;
		*v_RX_total_consumed_size = local_RX_total_consumed_size;
	}

	// exit
	if (sem_post(&thread_sem))
		perror("sem_post failed:");

	return 0;
}
