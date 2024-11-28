// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
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
static void *ll_table;

int pci_dma_mem_init(uint32_t dma_base_addr, uint32_t edma_reg_size, uint32_t ll_base_addr, uint32_t nb_desc, int sg_en)
{

	if (memfd == -1)
		memfd = open("/dev/mem", O_RDWR | O_SYNC);
	if (memfd == -1) {
		printf("Can't open /dev/mem.\n");
		return -1;
	}

	printf("/dev/mem opened.\n");

	if (mapped_base == NULL) {
		mapped_base = mmap(NULL, edma_reg_size, PROT_READ | PROT_WRITE, MAP_SHARED,
				memfd, dma_base_addr);
		if (mapped_base == (void *) -1) {
			printf("Can't map the pci edma memory to user space.\n");
			return -1;
		}
		printf("Memory mapped at address %p.\n", mapped_base);
	}

	if (sg_en) {
		if (ll_table == NULL) {
			ll_table = mmap(NULL, sizeof(edma_ll_table)*nb_desc,
					PROT_READ | PROT_WRITE, MAP_SHARED,
					memfd, ll_base_addr);
			if (ll_table == (void *) -1) {
				printf("Can't map the memory to user space.\n");
				return -1;
			}
			printf("Memory mapped ll_table address %p.\n", ll_table);
		}
	}

	return 0;
}

void pci_dma_mem_deinit(uint32_t edma_reg_size, uint32_t nb_desc)
{
	if (memfd == -1)
		return;

	/* unmap the memory before exiting */
	if (mapped_base) {
		if (munmap(mapped_base, edma_reg_size) == -1)
			printf("can't unmap memory from user space.\n");
		mapped_base = NULL;
	}
	if (ll_table) {
		if (munmap(ll_table, sizeof(edma_ll_table)*nb_desc) == -1)
			printf("can't unmap memory from user space.\n");
		ll_table = NULL;
	}

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

void pci_dma_read(uint32_t pci_src, uint32_t ddr_dst, uint32_t size, uint32_t ch_num, int sg_en)
{
	uint32_t chan_ctx_base_reg = -1;
	uint32_t ll_read_err = 0;

	chan_ctx_base_reg = (DMA_CHANNEL_CTXT_OFF_REG * ch_num)
			+ DMA_CHANNEL_READ_CTXT_OFF_REG;
	write_reg(DMA_READ_ENABLE_REG, DMA_EN);
	write_reg(DMA_READ_PW_ENABLE_BASE_REG, DMA_PW_EN);
	write_reg(DMA_READ_INT_MASK_REG, 0x0);
	if (sg_en) {
		ll_read_err = read_reg(DMA_READ_LL_ERR_EN);
		ll_read_err |= 0x00000001;
		write_reg(DMA_READ_LL_ERR_EN, ll_read_err);
		write_reg(chan_ctx_base_reg, DMA_LL_CONT_VAL);
		write_reg(chan_ctx_base_reg + DMA_LL_LOW_REG, pci_src);
		write_reg(chan_ctx_base_reg + DMA_LL_HIGH_REG, 0);
	} else {
		write_reg(chan_ctx_base_reg, DMA_CONT_VAL);
		write_reg(chan_ctx_base_reg + DMA_SIZE_REG, size);
		write_reg(chan_ctx_base_reg + DMA_SRC_LOW_REG, pci_src);
		write_reg(chan_ctx_base_reg + DMA_SRC_HIGH_REG, 0);
		write_reg(chan_ctx_base_reg + DMA_DST_LOW_REG, ddr_dst);
		write_reg(chan_ctx_base_reg + DMA_DST_HIGH_REG, 0);
	}
	write_reg(DMA_READ_DOORBELL_REG, ch_num - 1);
}

void pci_dma_write(uint32_t ddr_src, uint32_t pci_dst, uint32_t size, uint32_t ch_num, int sg_en)
{
	uint32_t chan_ctx_base_reg = -1;
	uint32_t ll_write_err = 0;

	chan_ctx_base_reg = (DMA_CHANNEL_CTXT_OFF_REG * ch_num);
	write_reg(DMA_WRITE_ENABLE_REG, DMA_EN);
	write_reg(DMA_WRITE_PW_ENABLE_BASE_REG, DMA_PW_EN);
	write_reg(DMA_WRITE_INT_MASK_REG, 0x0);
	if (sg_en) {
		ll_write_err = read_reg(DMA_WRITE_LL_ERR_EN);
		ll_write_err |= 0x00000001;
		write_reg(DMA_WRITE_LL_ERR_EN, ll_write_err);
		write_reg(chan_ctx_base_reg, DMA_LL_CONT_VAL);
		write_reg(chan_ctx_base_reg + DMA_LL_LOW_REG, ddr_src);
		write_reg(chan_ctx_base_reg + DMA_LL_HIGH_REG, 0);
	} else {
		write_reg(chan_ctx_base_reg, DMA_CONT_VAL);
		write_reg(chan_ctx_base_reg + DMA_SIZE_REG, size);
		write_reg(chan_ctx_base_reg + DMA_SRC_LOW_REG, ddr_src);
		write_reg(chan_ctx_base_reg + DMA_SRC_HIGH_REG, 0);
		write_reg(chan_ctx_base_reg + DMA_DST_LOW_REG, pci_dst);
		write_reg(chan_ctx_base_reg + DMA_DST_HIGH_REG, 0);
	}
	write_reg(DMA_WRITE_DOORBELL_REG, ch_num - 1);
}

static void fill_ll_table(edma_ll_table *List, uint32_t src_addr, uint32_t dst_addr, uint32_t size )
{
	List->control = 0x00000001;
	List->transfer_size = size;
	List->sar.lsb = (uint32_t)lower_32_bits(src_addr);
	List->sar.msb = upper_32_bits(src_addr);
	List->dar.lsb = (uint32_t)lower_32_bits(dst_addr);;
	List->dar.msb = upper_32_bits(dst_addr);
}

static void edmaFillLLEntries(edma_ll_table *edma_ll, host_desc_t *host_desc, mdm_desc_t *mdm_desc, uint32_t nb_desc, int dir)
{
	if (!edma_ll)
		return;

	memset(edma_ll, 0, (sizeof(edma_ll_table) * nb_desc));

	for (uint32_t i = 0; i < nb_desc; i++) {
		if (!dir){
			fill_ll_table(edma_ll, host_desc[i].addr,
					mdm_desc[i].addr, host_desc[i].size);
		} else {
			fill_ll_table(edma_ll, mdm_desc[i].addr,
					host_desc[i].addr, host_desc[i].size);
		}

		edma_ll++;
	}
}

void pci_dma_sg_configure(host_desc_t *host_desc, mdm_desc_t *mdm_desc, uint32_t nb_desc, int dir)
{
	static void *ll_src_addr[SRC_LL_NUM];
	static void *llSrc;
	size_t srclenArr[SRC_LL_NUM];
	uint32_t used_size = 0;
	edma_ll_table *sgLLTableHead;

	sgLLTableHead = (edma_ll_table *)(ll_table);
	memset(sgLLTableHead, 0, sizeof(edma_ll_table) * nb_desc);

	edmaFillLLEntries(sgLLTableHead, host_desc, mdm_desc, nb_desc, dir);
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
