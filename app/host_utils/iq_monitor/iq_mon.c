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

uint32_t *v_iqflood_ddr_addr;
uint32_t *v_scratch_ddr_addr;
uint32_t *v_ocram_addr;
uint32_t *v_vspa_dmem_proxy_ro;
t_stats *host_stats;
t_tx_ch_host_proxy *tx_vspa_proxy_ro;
t_rx_ch_host_proxy *rx_vspa_proxy_ro;
uint32_t *v_tx_vspa_proxy_wo;
uint32_t *v_rx_vspa_proxy_wo;

uint32_t *BAR0_addr;
uint32_t *BAR1_addr;
uint32_t *BAR2_addr;
uint32_t *PCIE1_addr;

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

//	printf("\n map iqflood_ddr :");
//	la9310_hexdump(v_iqflood_ddr_addr,64);

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

	/* use last 256 bytes of iqflood as shared vspa dmem proxy , vspa will write mirrored dmem value to avoid PCI read from host */
	v_vspa_dmem_proxy_ro = (uint32_t *)(v_iqflood_ddr_addr + (mi.iqflood.size - VSPA_DMEM_PROXY_SIZE)/4);
	host_stats =  &(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy_ro)->host_stats);
	rx_vspa_proxy_ro =  &(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy_ro)->rx_state_readonly[0]);
	tx_vspa_proxy_ro =  &(((t_vspa_dmem_proxy *)v_vspa_dmem_proxy_ro)->tx_state_readonly);

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
}

void sigusr1_process(int sig)
{
    fflush(stdout);
}

void print_cmd_help(void)
{
    fprintf(stderr, "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    fprintf(stderr, "\n|    iq_monitor");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| ./iq_monitor");
    fprintf(stderr, "\n|");
    fprintf(stderr, "\n| Trace");
    fprintf(stderr, "\n|\t-h	help");
    fprintf(stderr, "\n|\t-c	clear host stats");
    fprintf(stderr, "\n|\t-v	version");
    fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

/* need following vspa symbols to be exported (vspa_exported_symbols.h)*/
int main(int argc, char *argv[])
{
	int32_t c, i, ret;
	command_e command = OP_MONITOR;
	struct stat buffer;
    int32_t forceFifoSize = 0;

   /* command line parser */
   while ((c = getopt(argc, argv, "hfrts:mdvxDMc")) != EOF)
   {
		switch (c) {
			case 'h': // help
				print_cmd_help();
				exit(1);
			break;
			case 'c':
				command = OP_CLEAR_STATS;
			break;
			default:
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
	if (ret != 0) {
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

	if (command == OP_CLEAR_STATS) {
		for (i = 0; i < sizeof(t_stats)/4; i++) {
			//dccivac((uint32_t*)(host_stats)+i);
			*((uint32_t*)(host_stats)+i) = 0;
		}
	}

	if (command == OP_MONITOR) {
		monitor_vspa_stats();
	}

	return 0;
}



