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

#include "iq_app.h"
#include "imx8-host.h"
#include "l1-trace-host.h"
#include "lib_iqplayer_api.h"

void *process_ant_tx_streaming_app(void *arg);
void *process_ant_rx_streaming_app(void *arg);

volatile uint32_t running;

uint32_t *v_iqflood_ddr_addr;
uint32_t *v_scratch_ddr_addr;
uint32_t *v_la9310_bar2;

uint32_t RxChanID;

uint32_t app_ddr_file_start;
uint32_t app_ddr_file_size;

uint32_t modem_ddr_fifo_start;
uint32_t modem_ddr_fifo_size;

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

static int map_physical_regions(void)
{
	int devmem_fd, i;

   /*
	* map memory regions
	*/

	devmem_fd = open("/dev/mem", O_RDWR);
	if (-1 == devmem_fd) {
		perror("/dev/mem open failed");
		return -1;
	}

	v_scratch_ddr_addr = mmap(NULL,  mi.scratchbuf.size, PROT_READ | PROT_WRITE,
	MAP_SHARED, devmem_fd,  mi.scratchbuf.host_phy_addr);
	if (v_scratch_ddr_addr == MAP_FAILED) {
		perror("Mapping v_scratch_ddr_addr buffer failed\n");
		return -1;
	}

	v_iqflood_ddr_addr = mmap(NULL, mi.iqflood.size, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, mi.iqflood.host_phy_addr);
	if (v_iqflood_ddr_addr == MAP_FAILED) {
		perror("Mapping v_iqflood_ddr_addr buffer failed\n");
		return -1;
	}

	v_la9310_bar2 = mmap(NULL, BAR2_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, devmem_fd, BAR2_ADDR);
	if (v_la9310_bar2 == MAP_FAILED) {
		perror("Mapping v_la9310_bar2 buffer failed\n");
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
}

void sigusr1_process(int sig)
{
    fflush(stdout);
	print_host_trace();
}

void print_cmd_help(void)
{
    fprintf(stderr, "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    fprintf(stderr, "\n|    iq_app");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n|\t-h	help");
    fprintf(stderr, "\n|\t-t	Tx mode");
    fprintf(stderr, "\n|\t-r	Rx mode");
    fprintf(stderr, "\n|\t-c	RX chanID 0-3 (0 default)");
    fprintf(stderr, "\n|\t-a    <poffset> <size>  Buffer in DDR (in IQFLOOD region)");
    fprintf(stderr, "\n|\t-f    <poffset> <size>  Fifo in DDR (in IQFLOOD region)");
    fprintf(stderr, "\n|\t-v	version");
    fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

int main(int argc, char *argv[])
{
	int32_t c, i, ret;
	command_e command = 0;
	struct stat buffer;
    int32_t forceFifoSize = 0;

	if (argc <= 1) {
		print_cmd_help();
		exit(1);
	}
	signal(SIGINT, terminate_process);
	signal(SIGTERM, terminate_process);
	signal(SIGUSR1, sigusr1_process);

   /* command line parser */
   while ((c = getopt(argc, argv, "htra:f:c:")) != EOF)
   {
		switch (c) {
			case 'h':
				print_cmd_help();
				exit(1);
			break;
			case 't':
				command = OP_TX_ONLY;
			break;
			case 'r':
				command = OP_RX_ONLY;
			break;
			case 'a':
				app_ddr_file_start = strtoull(argv[optind-1], 0, 0);
				app_ddr_file_size = strtoul(argv[optind], 0, 0);
			break;
			case 'f':
				modem_ddr_fifo_start = strtoull(argv[optind-1], 0, 0);
				modem_ddr_fifo_size = strtoul(argv[optind], 0, 0);
			break;
			case 'c':
				RxChanID = strtoull(argv[optind-1], 0, 0);
			break;
			default:
				print_cmd_help();
				exit(1);
			break;

		}
	}

	/* Check modem is on */
	if (stat("/sys/shiva/shiva_status", &buffer) != 0) {
		perror("la9310 shiva driver not started");
		exit(EXIT_FAILURE);
	}

#ifdef IQMOD_RX_1R
	if (RxChanID != 0) {
		perror("Rx chan not supported, app compiled for 1T1R");
		exit(EXIT_FAILURE);
	}
#endif

	running = 1;

	/* Get modem info*/
    ret = get_modem_info(0);
    if (ret != 0) {
	perror("Fail to get modem_info \r\n");
		exit(EXIT_FAILURE);
    }

	/* map memory regions */
	if (map_physical_regions()) {
			perror("map_physical_regions failed:");
			exit(EXIT_FAILURE);
	}

	ret = iq_player_init(v_iqflood_ddr_addr, mi.iqflood.size, v_la9310_bar2);
	if (!ret) {
		printf("\n TX : iq_player_init failed\n");
		fflush(stdout);
		return 0;
	}

	/* start Tx/Rx */
	if (command == OP_TX_ONLY) {
		process_ant_tx_streaming_app(NULL);
	}
	if (command == OP_RX_ONLY) {
		process_ant_rx_streaming_app(NULL);
	}

	//print_host_trace();

	return 0;
}

void *process_ant_tx_streaming_app(void *arg)
{
	uint32_t ddr_rd_offset = 0, size_sent = 0;
	void *ddr_src;
	int ret;

    // init tx channel
	ret = iq_player_init_tx(modem_ddr_fifo_start, modem_ddr_fifo_size);
	if (!ret) {
		printf("\n TX : iq_player_init_tx failed\n");
		fflush(stdout);
		return 0;
	}

	// start feeding tx fifo
	while (running) {
		// prepare next transmit
		ddr_src = (void *)((uint64_t)v_iqflood_ddr_addr + app_ddr_file_start + ddr_rd_offset);
		if (app_ddr_file_size-ddr_rd_offset > modem_ddr_fifo_size) {
				size_sent = iq_player_send_data(ddr_src, modem_ddr_fifo_size);
		} else {
				size_sent = iq_player_send_data(ddr_src, app_ddr_file_size-ddr_rd_offset);
		}
		// update pointers
		ddr_rd_offset += size_sent;
		if (ddr_rd_offset >= app_ddr_file_size) {
			ddr_rd_offset = 0;
		}
	}

	return 0;
}

void *process_ant_rx_streaming_app(void *arg)
{
	uint32_t ddr_wr_offset = 0, size_received = 0;
	void *ddr_dst;
	int ret;

    // init tx channel
	ret = iq_player_init_rx(RxChanID, modem_ddr_fifo_start, modem_ddr_fifo_size);
	if (!ret) {
		printf("\n RX : iq_player_init_rx failed\n");
		fflush(stdout);
		return 0;
	}

	// start emptying rx fifo
	while (running) {
		// prepare next transmit
		ddr_dst = (void *)((uint64_t)v_iqflood_ddr_addr + app_ddr_file_start + ddr_wr_offset);
		if (app_ddr_file_size-ddr_wr_offset > modem_ddr_fifo_size) {
				size_received = iq_player_receive_data(RxChanID, ddr_dst, modem_ddr_fifo_size);
		} else {
				size_received = iq_player_receive_data(RxChanID, ddr_dst, app_ddr_file_size-ddr_wr_offset);
		}
		// update pointers
		ddr_wr_offset += size_received;
		if (ddr_wr_offset >= app_ddr_file_size) {
			ddr_wr_offset = 0;
		}
	}

	return 0;
}

