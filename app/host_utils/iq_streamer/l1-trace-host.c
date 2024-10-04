// =============================================================================
//! @file       l1-trace-host.c
//! @brief      L1 trace function
//! @author     NXP Semiconductors.
//! @copyright  Copyright 2023-2024 NXP
// =============================================================================
/*
*  NXP Confidential. This software is owned or controlled by NXP and may only be used strictly
*  in accordance with the applicable license terms. By expressly accepting
*  such terms or by downloading, installing, activating and/or otherwise using
*  the software, you are agreeing that you have read, and that you agree to
*  comply with and are bound by, such license terms. If you do not agree to
*  be bound by the applicable license terms, then you may not retain,
*  install, activate or otherwise use the software.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "l1-trace.h"
#include "vspa_trace_enum.h"
#include "vspa_exported_symbols.h"
#include "iq_streamer.h"

//#define ccnt_read() ((uint64_t)((*_VSPA_cyc_count_msb)&0x7FFFFFFF)*0x100000000 + *_VSPA_cyc_count_lsb)
uint64_t ccnt_read(void)
{
	uint64_t time;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time));
	return time;
}

int rte_get_tsc_hz_init=0; 

static inline uint64_t rte_get_tsc_hz(void)
{
    FILE *fp;
    char path[1035], *stopstring;
    uint64_t val = 0;

	if(0==rte_get_tsc_hz_init){
		/*Get CPU freq */
		fp = popen("/sys/kernel/debug/clk/arm_a*_core/clk_rate", "r");
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
		//printf("%s: CPU freq %ld \n", __func__, val);
	}
	
    return rte_get_tsc_hz_init;
}

l1_trace_data_t l1_trace_data[L1_TRACE_SIZE] __attribute__ ((aligned(64)));
uint32_t l1_trace_index;
volatile uint32_t l1_trace_disable;

void l1_trace_clear(void)
{
	for (l1_trace_index = 0; l1_trace_index < L1_TRACE_SIZE; l1_trace_index++) {
	    l1_trace_data[l1_trace_index].cnt = 0;
	    l1_trace_data[l1_trace_index].msg = 0;
	    l1_trace_data[l1_trace_index].param = 0;
	}
	l1_trace_index = 0;
}

void l1_trace(uint32_t msg, uint32_t param)
{
	if (l1_trace_disable) return;

    l1_trace_data[l1_trace_index].cnt = ccnt_read();
    l1_trace_data[l1_trace_index].msg = msg;
    l1_trace_data[l1_trace_index].param = param;
    l1_trace_index++;

    if (l1_trace_index >= L1_TRACE_SIZE) {
	l1_trace_index = 0;
    }
}

void print_host_trace(void)
{
	uint32_t i, j, k;
	uint32_t STATS_MAX = sizeof(l1_trace_data)/sizeof(l1_trace_data_t);
	l1_trace_data_t *entry;
	uint64_t prev;

	l1_trace_disable = 1;

	prev = l1_trace_data[l1_trace_index].cnt;
	for (i = 0, j = l1_trace_index; i < STATS_MAX; i++, j++) {
		if (j >= STATS_MAX) j = 0;
		entry = (l1_trace_data_t *)l1_trace_data + j;
		k = 0;
		while (l1_trace_code[k].msg != 0xffff) { if (l1_trace_code[k].msg == entry->msg) break; k++; }
		printf("\n%4d, counter: %016ld (+%08ld ns), param: 0x%08x, (0x%03x)%s", i, entry->cnt, (entry->cnt-prev)*1000000000/rte_get_tsc_hz(), entry->param, entry->msg, l1_trace_code[k].text);
		prev = entry->cnt;
	}
	printf("\n");

	l1_trace_disable = 0;

	return;
}

//VSPA_EXPORT(l1_trace_data)
//VSPA_EXPORT(l1_trace_index)
//VSPA_EXPORT(l1_trace_disable)

// format
//146, counter: 0000045638347355, param: 0x0B003000, L1_TRACE_MSG_DMA_AXIQ_TX
#ifdef v_l1_trace_data
void print_vspa_trace(void)
{
	uint32_t i, j, k;
	uint32_t STATS_MAX = s_l1_trace_data/sizeof(l1_trace_data_t);
	l1_trace_data_t *entry;

	*(v_l1_trace_disable) = 1;

	for (i = 0, j =  *(v_l1_trace_index); i < STATS_MAX; i++, j++) {
		if (j >= STATS_MAX) j = 0;
		entry = (l1_trace_data_t *)v_l1_trace_data + j;
		k = 0;
		while (l1_trace_code[k].msg != 0xffff) { if (l1_trace_code[k].msg == entry->msg) break; k++; }
		printf("\n%4d, counter: %016ld, param: 0x%08x, (0x%03x)%s", i, entry->cnt, entry->param, entry->msg, l1_trace_code[k].text);
	}
	printf("\n");

	*(v_l1_trace_disable) = 0;

	return;
}
#else
void print_vspa_trace(void)
{
printf("\n VSPA trace disabled \n");
}
#endif


