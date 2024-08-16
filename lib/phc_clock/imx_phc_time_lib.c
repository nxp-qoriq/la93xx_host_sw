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

static int memfd;
static void *mapped_base;

#define CLK_1588_BASE_ADDRESS       0x4043C000
#define MAP_SIZE                    4096UL
#define MAP_MASK (MAP_SIZE - 1)
#define MAC_TIMESTAMP_CONTROL_OFFSET 0xB00 // Timestamp Control
#define MAC_SUB_SECOND_INCREMENT_OFFSET 0xB04 //Subsecond Increment
#define MAC_SYSTEM_TIME_SECONDS_OFFSET 0xB08 //System Time Seconds
#define MAC_SYSTEM_TIME_NANOSECONDS_OFFSET 0xB0C// System Time Nanoseconds
#define MAC_SYSTEM_TIME_SECONDS_UPDATE_OFFSET 0xB10 //System Time Seconds Update
#define MAC_SYSTEM_TIME_NANOSECONDS_UPDATE_OFFSET 0xB14 //System Time Nanoseconds Update
#define MAC_TIMESTAMP_ADDEND_OFFSET 0xB18 //Timestamp Addend
#define MAC_SYSTEM_TIME_HIGHER_WORD_SECONDS_OFFSET 0xB1C //System Time - Higher Word Seconds
#define MAC_TIMESTAMP_STATUS_OFFSET  0xB20 //Timestamp Status
#define MAC_TX_TIMESTAMP_STATUS_NANOSECONDS_OFFSET 0xB30 //Tranimit Timestamp Status Nanoseconds
#define MAC_TX_TIMESTAMP_STATUS_SECONDS_OFFSET 0xB34 //Transmit Timestamp Status Seconds

int init_1588(void)
{
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		return -1;
	}
	printf("/dev/mem opened.\n");

	mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, CLK_1588_BASE_ADDRESS);
	if (mapped_base == (void *) -1) {
		printf("Can't map the memory to user space.\n");
		return -1;
	}
	printf("Memory mapped at address %p.\n", mapped_base);

	return 0;
}

void deinit_1588(void)
{
	/* unmap the memory before exiting */
	if (munmap(mapped_base, MAP_SIZE) == -1) {
		printf("Can't unmap memory from user space.\n");
	}

	close(memfd);
	memfd = 0;
	mapped_base = NULL;
}

int tmr_cnt_read(uint32_t *seconds, uint32_t *nseconds)
{
	*seconds = *(volatile uint32_t *)(mapped_base + MAC_SYSTEM_TIME_SECONDS_OFFSET);
	*nseconds = *(volatile uint32_t *)(mapped_base + MAC_SYSTEM_TIME_NANOSECONDS_OFFSET);

	return 0;
}

int tmr_cnt_write(uint32_t seconds, uint32_t nseconds)
{
	*(volatile uint32_t *)(mapped_base + MAC_SYSTEM_TIME_NANOSECONDS_OFFSET) = nseconds;
	*(volatile uint32_t *)(mapped_base + MAC_SYSTEM_TIME_SECONDS_OFFSET) = seconds;

	return 0;
}

uint32_t tmr_read_reg(uint32_t reg_addr)
{
	uint32_t read_reg;

	read_reg = *(volatile uint32_t *)(mapped_base + reg_addr);

	return read_reg;
}
