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

#include "vspa_exported_symbols.h"
#include "iq_streamer.h"
#include "stats.h"
#include "la9310_regs.h"

uint64_t cycle_count_idle_start[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_idle_done[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_xfer_start[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_xfer_done[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_duration[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_max_duration[NUM_ANT][MAX_CYC_CNT];
uint64_t cycle_count_max_date[NUM_ANT][MAX_CYC_CNT];

uint64_t cycle_count_index[NUM_ANT]={0,0};
uint64_t cycle_count_max_index[NUM_ANT]={0,0};

uint32_t stats_locked=0;

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
        unsigned long start = p & ~(unsigned long)15;
        unsigned long end = (p + sz + 15) & ~(unsigned long)15;
        const unsigned char *c = ptr;

        __hexdump(start, end, p, sz, c);
}

char * VSPA_stat_string[]={
	"DMA_AXIQ_WRTIE\t",
	"DMA_AXIQ_READ\t", 
	"DMA_DDR_RD\t",
	"DMA_DDR_WR\t",
	"DMA_DDR_RD_UDR\t",
	"DMA_DDR_WR_OVR\t",
	"FIFO_TX_UDR\t",
	"FIFO_RX_OVR\t",
	"DMA_TX_CMD_UDR\t",
	"DMA_RX_CMD_OVR\t",
	"CONFIG_ERROR\t",
	"XFER_ERROR\t",
	"STATS_MAX"
	};
typedef enum
{
	STAT_DMA_AXIQ_WRTIE=0,          // 0x0
	STAT_DMA_AXIQ_READ,             // 0x1
	STAT_DMA_DDR_RD,                // 0x2
	STAT_DMA_DDR_WR,                // 0x3
	ERROR_DMA_DDR_RD_UNDERRUN,      // 0x4
	ERROR_DMA_DDR_WR_OVERRRUN,      // 0x5
	ERROR_AXIQ_FIFO_TX_UNDERRUN,    // 0x6
	ERROR_AXIQ_FIFO_RX_OVERRUN,     // 0x7
	ERROR_AXIQ_DMA_TX_CMD_UNDERRUN, // 0x8
	ERROR_AXIQ_DMA_RX_CMD_OVERRUN, // 0x9
	ERROR_DMA_CONFIG_ERROR,         // 0xA
	ERROR_DMA_XFER_ERROR,           // 0xB
	STATS_MAX
} stats_e;

//VSPA_EXPORT(g_stats)
//VSPA_EXPORT(l1_trace_data)
uint32_t vspa_stats[2][s_g_stats/4];
uint32_t vspa_stats_tab=0;
volatile uint32_t *_VSPA_DMA_regs,*_GP_IN0;
void print_vspa_stats(void)
{
	int i;
	uint32_t *cur_stats, *prev_stats;
	
	// pinpong tables to keep prev values
	if(vspa_stats_tab){ 
		vspa_stats_tab=0;
		cur_stats=vspa_stats[0];
		prev_stats=vspa_stats[1];
	}		
	else{ 
		vspa_stats_tab=1;
		cur_stats=vspa_stats[1];
		prev_stats=vspa_stats[0];
	}
	
	// Take snap shot
	for(i=0;i<STATS_MAX;i++){
		cur_stats[i]=*(v_g_stats+i);
	}
	for(i=0;i<STATS_MAX;i++){
		printf("\n %s \t0x%08x \t (%08d)  ",VSPA_stat_string[i],cur_stats[i],(uint32_t)(cur_stats[i]-prev_stats[i]));
//		if(i<4)
		printf("(%08d MB/s)  ",(uint32_t)((uint64_t)(cur_stats[i]-prev_stats[i])*2048/1000000));
	}
	printf("\n");

	_VSPA_DMA_regs= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + DMA_DMEM_PRAM_ADDR);
	printf("VSPA DMA regs (IP reg 0xB0):\n");
	la9310_hexdump((void*)_VSPA_DMA_regs,0x30);
	_GP_IN0= (volatile uint32_t *)((uint64_t)BAR0_addr + VSPA_CCSR + GP_IN0);
	printf("VSPA AXI regs (GP_IN0-9 IPREG 0x580):\n");
	la9310_hexdump((void*)_GP_IN0,0x30);

	return ;
}

void monitor_vspa_stats(void)
{
    printf("\033[2J");
	while(running){
		printf("\033[H");
		print_vspa_stats();
		sleep(1);
	}
	return ;
}


void print_history(void )
{
	int i,k;
	stats_locked=1;
	for(k=0;k<NUM_ANT;k++){
		
		for(i=0;i<MAX_CYC_CNT;i++){
			printf("\n%02d:%02d: duration 0x%08lx , idle start 0x%08lx, end 0x%08lx, dma start 0x%08lx , end 0x%08lx"
				   ,k,i
				   ,cycle_count_duration[k][i]
				   ,cycle_count_idle_start[k][i],cycle_count_idle_done[k][i]
				   ,cycle_count_xfer_start[k][i],cycle_count_xfer_done[k][i]);
		}
		
		for(i=0;i<MAX_CYC_CNT;i++){
			if(cycle_count_max_duration[k][i]!=0)
			printf("\n%02d:%2d: max duration 0x%08lx , date 0x%08lx"
				   ,k,i
				   ,cycle_count_max_duration[k][i]
				   ,cycle_count_max_date[k][i]);
		}
	}
	printf("\n");
	stats_locked=0;
}

typedef struct l1_trace_data_s {
    uint64_t cnt;
    uint32_t msg;
    uint32_t param;
} l1_trace_data_t;

typedef struct l1_trace_code_s {
    uint32_t msg;
    char text[100];
} l1_trace_code_t;

#include "vspa_trace_enum.h"

//VSPA_EXPORT(l1_trace_data)
//VSPA_EXPORT(l1_trace_index)
//VSPA_EXPORT(l1_trace_disable)

// format
//146, counter: 0000045638347355, param: 0x0B003000, L1_TRACE_MSG_DMA_AXIQ_TX
void print_vspa_trace(void)
{
	uint32_t i,j,k;
	uint32_t STATS_MAX = s_l1_trace_data/sizeof(l1_trace_data_t);
	l1_trace_data_t* entry;

	*(v_l1_trace_disable)=1;

	for(i=0,j=*(v_l1_trace_index);i<STATS_MAX;i++,j++){
		if(j>=STATS_MAX) j=0;
		entry= (l1_trace_data_t*)v_l1_trace_data + j;
		k=0;
		while(l1_trace_code[k].msg!=0xffff){ if(l1_trace_code[k].msg==entry->msg) break; k++; }
		printf("\n%4d, counter: %016ld, param: 0x%08x, (0x%03x)%s",i,entry->cnt,entry->param,entry->msg,l1_trace_code[k].text);
	}
	printf("\n");

	*(v_l1_trace_disable)=0;

	return ;
}

