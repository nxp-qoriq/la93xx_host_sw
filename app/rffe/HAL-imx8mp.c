/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2025 NXP
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
 #include "HAL-imx8mp.h"


volatile unsigned int *gpio[7];
static int memfd = -1;
static void *mapped_gpio_base;
static void *ll_table;

 #define IMX8MP_GPIO_BASE 0x30200000
 #define IMX8MP_GPIO_SIZE 0x50000

/*
 *   HAL_GPIO
 */

int HAL_GPIO_Init(void)
{
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		return -1;
	}
	mapped_gpio_base = mmap(NULL, IMX8MP_GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
							memfd, IMX8MP_GPIO_BASE);
	if (mapped_gpio_base == (void *) -1) {
		printf("Can't map the gpio memory to user space.\n");
		return -1;
	}

	gpio[1] = (volatile unsigned int *) ((uint8_t *)mapped_gpio_base + 0);
	gpio[2] = (volatile unsigned int *) ((uint8_t *)mapped_gpio_base + 0x10000);
	gpio[3] = (volatile unsigned int *) ((uint8_t *)mapped_gpio_base + 0x20000);
	gpio[4] = (volatile unsigned int *) ((uint8_t *)mapped_gpio_base + 0x30000);
	gpio[5] = (volatile unsigned int *) ((uint8_t *)mapped_gpio_base + 0x40000);

	return 0;
}

int HAL_Init(void)
{
	HAL_GPIO_Init();
	return 0;
}

void rfnm_gpio_exit(void)
{
	if (mapped_gpio_base) {
		if (munmap(mapped_gpio_base, IMX8MP_GPIO_SIZE) == -1)
			printf("can't unmap memory from user space.\n");

		mapped_gpio_base = NULL;
	}
}

void SetPinOutput(GPIO_TypeDef port, uint16_t pin)
{
	int bank, num;

	bank = (pin >> GPIO_BANK) & 0xff;
	num = (pin >> GPIO_NUM) & 0xff;
	*(gpio[bank]+1) |= (1 << num);
}

void SetPinInput(GPIO_TypeDef port, uint16_t pin)
{
	int bank, num;

	bank = (pin >> GPIO_BANK) & 0xff;
	num = (pin >> GPIO_NUM) & 0xff;
	*(gpio[bank]+1) &= ~(1 << num);
}

int HAL_GPIO_WritePin(GPIO_TypeDef port, uint16_t pin, int value)
{
	int bank, num;

	bank = (pin >> GPIO_BANK) & 0xff;
	num = (pin >> GPIO_NUM) & 0xff;

	if (value) {
		*gpio[bank] |= (1 << num);
	} else {
		*gpio[bank] &= ~(1 << num);
	}

	//l1_trace(L1_TRACE_MSG_GPIO_WRITE, (pin<<16)+value);

	return 0;
}

int HAL_GPIO_ReadPin(GPIO_TypeDef port, uint16_t pin)
{
	int bank, num, value;

	bank = (pin >> GPIO_BANK) & 0xff;
	num = (pin >> GPIO_NUM) & 0xff;

	value = (0 != (*gpio[bank] & (1 << num)));
	//l1_trace(L1_TRACE_MSG_GPIO_READ, (pin<<16)+value);

	return(value);
}

/*
 *   HAL_TIME
 */

uint64_t rte_get_tsc_cycles(void)
{
	uint64_t time;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time));
	return time;
}

uint64_t rte_get_tsc_hz_init;
uint64_t cpufreq;

static inline uint64_t rte_get_tsc_hz(void)
{
	FILE *fp;
	char path[1035], *stopstring;
	uint64_t val = 0;

	if (rte_get_tsc_hz_init == 0) {
		/*Get CPU freq */
		fp = fopen("/sys/kernel/debug/clk/arm_a53_core/clk_rate", "r");
		if (fp == NULL) {
			printf("Failed to run command\n");
			exit(1);
		}

		/* Read the output a line at a time - output it. */
		while (fgets(path, sizeof(path), fp) != NULL) {
			val = strtoull(path, &stopstring, 10);
			//printf("\n%lx\n", val);
		}

		if (val == 0) {
		printf("default device not match, use the default freq.\n");
		rte_get_tsc_hz_init = 1600000000;
		} else {
		rte_get_tsc_hz_init = val;
		}

		//printf("%s: CPU freq %ld\n", __func__, val);
	}

    return rte_get_tsc_hz_init;
}

int HAL_TIM_Base_Init(void)
{
	cpufreq = rte_get_tsc_hz();
	return 0;
}

uint64_t start_time;
int HAL_TIM_Base_Start(void)
{
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(start_time));
	return 0;
}
int HAL_TIM_Base_Stop(void)
{
	start_time = 0;
	return 0;
}


// SCLK clock frequencies from 32 kHz to 26 MHz
//      i.e. T from 32 μs to 38.4 ns

int __HAL_TIM_GET_COUNTER_ns(void)
{
	uint64_t time, time_ns;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time));
	time = time-start_time;
	time_ns = (time * 1000000000)/cpufreq;
	return (int)(time_ns);
}


// cycle_count/cpufreq -> time in second
//  x 1000000 to get time in us
int __HAL_TIM_GET_COUNTER_us(void)
{
	uint64_t time, time_us;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time));
	time = time-start_time;
	time_us = (time * 1000000)/cpufreq;
	return (int)(time_us);
}


// cycle_count/cpufreq -> time in second
//  x 1000 to get time in ms
int HAL_Delay(int delay_ms)
{
	uint64_t time1, time2;
	uint64_t time1_ms, time2_ms;

	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time1));
	time1_ms = (time1 * 1000)/cpufreq;
	do {
		asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(time2));
		time2_ms = (time2 * 1000)/cpufreq;
	} while ((time2_ms-time1_ms) < delay_ms);
	return 0;
}

/*
 *  HAL_Trace
 */

l1_trace_host_data_t l1_trace_host_data[L1_TRACE_HOST_SIZE];
uint32_t l1_trace_index;
volatile uint32_t l1_trace_disable;


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

l1_trace_host_code_t l1_trace_host_code[] = {
{ 0x100, "L1_TRACE_MSG_GPIO_WRITE "},
{ 0x101, "L1_TRACE_MSG_GPIO_READ "},
{ 0xFFFF, "MAX_CODE_TRACE"}
};

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


