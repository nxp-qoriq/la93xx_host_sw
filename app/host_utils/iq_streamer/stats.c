/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2022-2024 NXP
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
#include "iq_streamer.h"
#include "la9310_regs.h"
#include "iqmod_rx.h"
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
#include "stats.h"


#define pr_info printf
void la9310_hexdump(const void *ptr, size_t sz);
void print_vspa_stats(void);
void monitor_vspa_stats(void);
void print_vspa_trace(void);

static inline void __hexdump(unsigned long start, unsigned long end,
		unsigned long p, size_t sz, const unsigned char *c)
{
	while (start < end) {
		unsigned int pos = 0;
		char buf[64];
		int nl = 0;

		pos += sprintf(buf + pos, "%08lx: ", start);
		do {
			if ((start < p) || (start >= (p + sz)))
				pos += sprintf(buf + pos, "..");
			else
				pos += sprintf(buf + pos, "%02x", *(c++));
			if (!(++start & 15)) {
				buf[pos++] = '\n';
				nl = 1;
			} else {
				nl = 0;
			if (!(start & 1))
				buf[pos++] = ' ';
			if (!(start & 3))
				buf[pos++] = ' ';
			}
		} while (start & 15);
		if (!nl)
			buf[pos++] = '\n';
		buf[pos] = '\0';
		pr_info("%s", buf);
	}
}
void la9310_hexdump(const void *ptr, size_t sz)
{
	unsigned long p = (unsigned long)ptr;
	unsigned long start = p & ~15UL;
	unsigned long end = (p + sz + 15) & ~15UL;
	const unsigned char *c = ptr;

	__hexdump(start, end, p, sz, c);
}

//VSPA_EXPORT(g_stats)
//VSPA_EXPORT(l1_trace_data)
//VSPA_EXPORT(l1_trace_data)
//VSPA_EXPORT(tx_busy_size)
//VSPA_EXPORT(rx_busy_size)

t_stats vspa_stats[2];
uint32_t vspa_stats_tab;
volatile uint32_t *_VSPA_DMA_regs, *_GP_IN0;
void print_vspa_stats(void)
{
	int i,j;
	t_stats *cur_stats, *prev_stats;

	// pinpong tables to keep prev values
	if (vspa_stats_tab) {
		vspa_stats_tab = 0;
		cur_stats = &vspa_stats[0];
		prev_stats = &vspa_stats[1];
	} else {
		vspa_stats_tab = 1;
		cur_stats = &vspa_stats[1];
		prev_stats = &vspa_stats[0];
	}

	// Take snap shot
	for (i = 0; i < s_g_stats/4; i++) {
		((uint32_t *)cur_stats)[i] =  *(v_g_stats+i);
	}
	printf("\nRX stats :");
	for (i = 0; i < STATS_RX_MAX; i++) {
		printf("\n %s",VSPA_stat_rx_string[i]);
		for (j = 0; j < RX_NUM_CHAN; j++) {
			uint32_t cur_val=*((uint32_t*)(cur_stats->rx_stats[j])+i);
			uint32_t prev_val=*((uint32_t*)(prev_stats->rx_stats[j])+i);
			printf("\t0x%08x", cur_val);
			if (i <= STAT_DMA_AXIQ_READ)
				printf("(%08d MB/s)", (uint32_t)((uint64_t)(cur_val-prev_val)*RX_DMA_TXR_size*4/1000000));
			else if (i < ERROR_DMA_DDR_WR_OVERRUN)
				printf("(%08d MB/s)", (uint32_t)((uint64_t)(cur_val-prev_val)*RX_DMA_TXR_size*4/1000000/RX_DECIM));
			else
				printf("               ");
		}
	}
	printf("\nTX stats :");
	for (i = 0; i < STATS_TX_MAX; i++) {
		uint32_t cur_val=*((uint32_t*)(cur_stats->tx_stats)+i);
		uint32_t prev_val=*((uint32_t*)(prev_stats->tx_stats)+i);
		printf("\n %s",VSPA_stat_tx_string[i]);
		printf("\t0x%08x", cur_val);
		if (i < ERROR_DMA_DDR_RD_UNDERRUN)
			printf("(%08d MB/s)", (uint32_t)((uint64_t)(cur_val-prev_val)*TX_DMA_TXR_size*4/1000000));
		else
			printf("               ");
	}
	printf("\nGlobal stats :");
	for (i = 0; i < STATS_GBL_MAX; i++) {
		uint32_t cur_val=*((uint32_t*)(cur_stats->gbl_stats)+i);
		uint32_t prev_val=*((uint32_t*)(prev_stats->gbl_stats)+i);
		printf("\n %s",VSPA_stat_gbl_string[i]);
		printf("\t0x%08x", cur_val);
		if (i < ERROR_DMA_CONFIG_ERROR)
			printf("(%08d MB/s)  ", (uint32_t)((uint64_t)(cur_val-prev_val)*RX_DMA_TXR_size*4/1000000));
		else
			printf("               ");
	}

	printf("\n");
	printf("\nTxFifo: %d/%d", *(v_tx_busy_size)/TX_DDR_STEP, TX_NUM_BUF);
	printf("\nRxFifo: %d/%d", *(v_rx_busy_size)/RX_DDR_STEP, RX_NUM_BUF);
	printf("\n");

	_VSPA_DMA_regs = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + DMA_DMEM_PRAM_ADDR);
	printf("VSPA DMA regs (IP reg 0xB0):\n");
	la9310_hexdump((void *)_VSPA_DMA_regs, 0x30);
	_GP_IN0 = (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + GP_IN0);
	printf("VSPA AXI regs (GP_IN0-9 IPREG 0x580):\n");
	la9310_hexdump((void *)_GP_IN0, 0x30);
	printf("\n");

	return;
}

void monitor_vspa_stats(void)
{
    printf("\033[2J");
	while (running) {
		printf("\033[H");
		print_vspa_stats();
		sleep(1);
	}
	return;
}


