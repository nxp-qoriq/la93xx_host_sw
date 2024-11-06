// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "imx8_edma.h"
#include <imx_edma_api.h>

static int memfd = -1;
static void *mapped_base;

int init_mem(uint32_t dma_base_address, uint32_t edma_reg_size)
{
	memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		return -1;
	}
	printf("/dev/mem opened.\n");

	mapped_base = mmap(NULL, edma_reg_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			memfd, dma_base_address);
	if (mapped_base == (void *) -1) {
		printf("Can't map the pci edma memory to user space.\n");
		return -1;
	}
	printf("Memory mapped at address %p.\n", mapped_base);

	return 0;
}

void deinit_mem(uint32_t edma_reg_size)
{
	if (memfd == -1)
		return;

	/* unmap the memory before exiting */
	if (munmap(mapped_base, edma_reg_size) == -1)
		printf("Can't unmap memory from user space.\n");

	close(memfd);
	memfd = -1;
	mapped_base = NULL;
}

static inline int write_reg(uint32_t reg_addr, uint32_t val)
{
	*(volatile uint32_t *)(mapped_base + reg_addr) = val;

	return 0;
}

static inline uint32_t read_reg(uint32_t reg_addr)
{
	uint32_t read_reg;

	read_reg = *(volatile uint32_t *)(mapped_base + reg_addr);

	return read_reg;
}

void pci_dma_read(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ch_num)
{
	uint32_t chan_ctx_base_reg = -1;

	chan_ctx_base_reg = (DMA_CHANNEL_CTXT_OFF_REG * ch_num)
			+ DMA_CHANNEL_READ_CTXT_OFF_REG;
	write_reg(DMA_READ_ENABLE_REG, DMA_EN);
	write_reg(DMA_READ_PW_ENABLE_BASE_REG, DMA_PW_EN);
	write_reg(DMA_READ_INT_MASK_REG, 0x0);
	write_reg(chan_ctx_base_reg, DMA_CONT_VAL);
	write_reg(chan_ctx_base_reg + DMA_SIZE_REG, size);
	write_reg(chan_ctx_base_reg + DMA_SRC_LOW_REG, pci_src);
	write_reg(chan_ctx_base_reg + DMA_SRC_HIGH_REG, 0);
	write_reg(chan_ctx_base_reg + DMA_DST_LOW_REG, ddr_dst);
	write_reg(chan_ctx_base_reg + DMA_DST_HIGH_REG, 0);
	write_reg(DMA_READ_DOORBELL_REG, ch_num - 1);
}

void pci_dma_write(uint32_t ddr_src, uint32_t pci_dst, uint32_t size, uint32_t ch_num)
{
	uint32_t chan_ctx_base_reg = -1;

	chan_ctx_base_reg = (DMA_CHANNEL_CTXT_OFF_REG * ch_num);
	write_reg(DMA_WRITE_ENABLE_REG, DMA_EN);
	write_reg(DMA_WRITE_PW_ENABLE_BASE_REG, DMA_PW_EN);
	write_reg(DMA_WRITE_INT_MASK_REG, 0x0);
	write_reg(chan_ctx_base_reg, DMA_CONT_VAL);
	write_reg(chan_ctx_base_reg + DMA_SIZE_REG, size);
	write_reg(chan_ctx_base_reg + DMA_SRC_LOW_REG, ddr_src);
	write_reg(chan_ctx_base_reg + DMA_SRC_HIGH_REG, 0);
	write_reg(chan_ctx_base_reg + DMA_DST_LOW_REG, pci_dst);
	write_reg(chan_ctx_base_reg + DMA_DST_HIGH_REG, 0);
	write_reg(DMA_WRITE_DOORBELL_REG, ch_num - 1);
}

void pci_dma_tx_reset(void)
{
	write_reg(DMA_WRITE_ENABLE_REG, !DMA_EN);
	write_reg(DMA_WRITE_ENABLE_REG, DMA_EN);
}

void pci_dma_rx_reset(void)
{
	write_reg(DMA_READ_ENABLE_REG, !DMA_EN);
	write_reg(DMA_READ_ENABLE_REG, DMA_EN);
}

int pci_dma_completed(int ch_num, int dir)
{
	uint32_t chanStatus = 0;
	uint32_t chan_ctx_base_reg = -1;

	chan_ctx_base_reg = (DMA_CHANNEL_CTXT_OFF_REG * ch_num)
			+ DMA_CHANNEL_READ_CTXT_OFF_REG * dir;
	chanStatus = read_reg(chan_ctx_base_reg);
	if ((chanStatus & 0x40) != 0x40)
		return -1;

	if ((chanStatus & 0x60) != 0x60)
		return -2;

	return 0;
}
