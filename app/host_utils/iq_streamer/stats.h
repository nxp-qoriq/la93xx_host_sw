/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2022-2024 NXP
*/

#ifndef __STATS_H__
#define __STATS_H__

#define pr_info printf
#define MAX_CYC_CNT 20

extern uint64_t cycle_count_idle_start[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_idle_done[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_xfer_start[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_xfer_done[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_duration[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_max_duration[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_max_date[NUM_ANT][MAX_CYC_CNT];
extern uint64_t cycle_count_index[NUM_ANT];
extern uint64_t cycle_count_max_index[NUM_ANT];
extern uint32_t stats_locked;

#define GET_VSPA_CYCLE_COUNT(table,ant,index) {\
	if(!stats_locked){\
	table[ant][index]=(uint64_t)((*_VSPA_cyc_count_msb)&0x7FFFFFFF)*0x100000000;\
	table[ant][index]+=*_VSPA_cyc_count_lsb;\
	}\
	}

void la9310_hexdump(const void *ptr, size_t sz);
void print_history(void );
void print_vspa_stats(void);
void monitor_vspa_stats(void);
void print_vspa_trace(void);


#endif