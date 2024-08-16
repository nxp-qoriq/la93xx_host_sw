// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/types.h>
#include <1588_phc_time_api.h>

#define MAC_VERSION_OFFSET 0x110
#define MAX_RD_WR_TEST   1000

int main(void)
{
	uint32_t read_reg, cnt;
	uint32_t seconds, nseconds;
	volatile  uint64_t prev, curr, delta;

	init_1588();

	read_reg = tmr_read_reg(MAC_VERSION_OFFSET);
	printf("MAC_VERSION REG  0x%X VAL 0x%X \r\n", MAC_VERSION_OFFSET, read_reg);
	read_reg = tmr_read_reg(0x11C);
	printf("VERSION REG  0x%X VAL 0x%X \r\n", MAC_VERSION_OFFSET, read_reg);

	/*Get pmu counter cpu cycles before test start */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(prev));
	for (cnt = 0; cnt < MAX_RD_WR_TEST; cnt++)
		tmr_cnt_read(&seconds, &nseconds);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr - prev;
	if (delta)
		printf("Read cpu cycles time taken for %d samples, one sample %ld (nano second)\r\n", MAX_RD_WR_TEST, ((delta * 10)/16) / MAX_RD_WR_TEST);
	else
		printf("Read No diff , something wrong\r\n");

	/*Get pmu counter cpu cycles before test start */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(prev));
	for (cnt = 0; cnt < MAX_RD_WR_TEST; cnt++)
		tmr_cnt_write(0xaabbccdd, 0x11223344);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr - prev;
	if (delta)
		printf("Write cpu cycles time taken for %d samples, one sample %ld (nano second)\r\n", MAX_RD_WR_TEST, ((delta * 10)/16) / MAX_RD_WR_TEST);
	else
		printf("Write No diff , something wrong\r\n");


	tmr_cnt_read(&seconds, &nseconds);
	printf("timer read seconds 0x%X nseconds  0x%X\r\n", seconds, nseconds);

	deinit_1588();

	return 0;
}


