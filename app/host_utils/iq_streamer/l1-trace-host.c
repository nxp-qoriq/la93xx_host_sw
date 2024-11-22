/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "l1-trace-host.h"

int rte_get_tsc_hz_init = 0; 
l1_trace_host_data_t l1_trace_host_data[L1_TRACE_HOST_SIZE]  __attribute__ ((aligned(64)));
uint32_t l1_trace_index;
volatile uint32_t l1_trace_disable;

l1_trace_host_code_t l1_trace_host_code[] = {
{ 0x100, "L1_TRACE_MSG_DMA_XFR "},
{ 0x101, "L1_TRACE_MSG_DMA_CFGERR "},
{ 0x102, "L1_TRACE_MSG_DMA_DDR_RD_START "},
{ 0x103, "L1_TRACE_MSG_DMA_DDR_RD_COMP "},
{ 0x104, "L1_TRACE_MSG_DMA_DDR_RD_UNDERRUN "},
{ 0x105, "L1_TRACE_MSG_DMA_DDR_WR_START "},
{ 0x106, "L1_TRACE_MSG_DMA_DDR_WR_COMP "},
{ 0x107, "L1_TRACE_MSG_DMA_DDR_WR_OVERRUN "},
{ 0xFFFF, "MAX_CODE_TRACE"}
};

/*
 *   PMC counter
 */

uint64_t rte_get_tsc_cycles(void)
{
	uint64_t time;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time));
	return time;
}

static inline uint64_t rte_get_tsc_hz(void)
{
    FILE *fp;
    char path[1035], *stopstring;
    uint64_t val = 0;

	if (rte_get_tsc_hz_init == 0) {
		/*Get CPU freq */
		fp = popen("/sys/kernel/debug/clk/arm_a53_core/clk_rate", "r");
		if (fp == NULL) {
			printf("Failed to run command\n");
			exit(1);
		}

		/* Read the output a line at a time - output it. */
		while (fgets(path, sizeof(path), fp) != NULL) {
			rte_get_tsc_hz_init = strtoull(path, &stopstring, 16);
			//printf("\n%lx\n", rte_get_tsc_hz_init);
		}

		if (val == 0) {
		printf("default device not match, use the default freq.\n");
		rte_get_tsc_hz_init = 1600000000;
		}
		//printf("%s: CPU freq %ld\n", __func__, val);
	}

    return rte_get_tsc_hz_init;
}


#if L1_TRACE

void l1_trace_host_clear(void)
{
	for (l1_trace_index = 0; l1_trace_index < L1_TRACE_HOST_SIZE; l1_trace_index++) {
	    l1_trace_host_data[l1_trace_index].cnt = 0;
	    l1_trace_host_data[l1_trace_index].msg = 0;
	    l1_trace_host_data[l1_trace_index].param = 0;
	}
	l1_trace_index = 0;
}

void l1_trace(uint32_t msg, uint32_t param)
{
	if (l1_trace_disable) return;

    l1_trace_host_data[l1_trace_index].cnt = rte_get_tsc_cycles();
    l1_trace_host_data[l1_trace_index].msg = msg;
    l1_trace_host_data[l1_trace_index].param = param;
    l1_trace_index++;

    if (l1_trace_index >= L1_TRACE_HOST_SIZE) {
	l1_trace_index = 0;
    }
}

#else // L1_TRACE
inline void l1_trace_clear(void)
{
}

inline void l1_trace(uint32_t msg, uint32_t param)
{
}
#endif // L1_TRACE

void print_host_trace(void)
{
	uint32_t i, j, k;
	uint32_t STATS_MAX = sizeof(l1_trace_host_data)/sizeof(l1_trace_host_data_t);
	l1_trace_host_data_t *entry;
	uint64_t prev;

	l1_trace_disable = 1;

	printf("\n host_trace:");

	prev = l1_trace_host_data[l1_trace_index].cnt;
	for (i = 0, j = l1_trace_index; i < STATS_MAX; i++, j++) {
		if (j >= STATS_MAX) j = 0;
		entry = (l1_trace_host_data_t *)l1_trace_host_data + j;
		k = 0;
		while (l1_trace_host_code[k].msg != 0xffff) { if (l1_trace_host_code[k].msg == entry->msg) break; k++; }
		printf("\n%4d, counter: %016ld (+%08ld ns), param: 0x%08x, (0x%03x)%s", i, entry->cnt, (entry->cnt-prev)*1000000000/rte_get_tsc_hz(), entry->param, entry->msg, l1_trace_host_code[k].text);
		prev = entry->cnt;
	}
	printf("\n");

	l1_trace_disable = 0;

	return;
}

