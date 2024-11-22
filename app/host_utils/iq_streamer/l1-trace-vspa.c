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
#include "l1-trace.h"
#include "vspa_trace_enum.h"

#ifdef IQMOD_RX_2R
#include "vspa_exported_symbols_2R.h"
#else
#ifdef IQMOD_RX_4R
#include "vspa_exported_symbols_4R.h"
#else
#include "vspa_exported_symbols.h"
#endif
#endif

extern uint32_t *BAR2_addr;

//VSPA_EXPORT(l1_trace_data)
//VSPA_EXPORT(l1_trace_index)
//VSPA_EXPORT(l1_trace_disable)

#ifdef v_l1_trace_data
void print_vspa_trace(void)
{
	uint32_t i, j, k;
	uint32_t TRACE_SIZE = s_l1_trace_data/sizeof(l1_trace_data_t);
	l1_trace_data_t *entry;

	*(v_l1_trace_disable) = 1;

	for (i = 0, j =  *(v_l1_trace_index); i < TRACE_SIZE; i++, j++) {
		if (j >= TRACE_SIZE) j = 0;
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

extern uint32_t *v_ocram_addr;
void print_m7_trace(void)
{
	uint32_t i,k;
	uint32_t TRACE_SIZE = L1_TRACE_M7_SIZE;
	l1_trace_data_t* entry=(l1_trace_data_t*)(v_ocram_addr);
	for(i=0;i<TRACE_SIZE;i++){
		k=0;
		while(l1_trace_code[k].msg!=0xffff){ if(l1_trace_code[k].msg==entry->msg) break; k++; }
		printf("\n%4d, counter: %016ld, param: 0x%08x, (0x%03x)%s",i,entry->cnt,entry->param,entry->msg,l1_trace_code[k].text);
		entry++;
	}
	printf("\n");
	return ;
}

