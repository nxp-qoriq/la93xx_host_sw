// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2024 NXP

#include <stdint.h>
#include <stdio.h>
#include<fcntl.h>
#include<sys/mman.h>
#include <stdlib.h>
#include <linux/types.h>
#include <unistd.h>
#include "imx_ecspi.h"
#include "diora_ecspi_api.h"

void *ecspi_handle[IMX8MP_ECSPI_MAX_DEVICES] = {NULL, NULL, NULL};
static int ecspi_fd[IMX8MP_ECSPI_MAX_DEVICES] = {-1, -1, -1};
static uint32_t ecspi_iomux_state[IMX8MP_ECSPI_MAX_DEVICES] = {0, 0, 0};
static uint32_t ecspi_store_org_ccm_clk_state[IMX8MP_ECSPI_MAX_DEVICES] = {0, 0, 0};
static uint32_t ecspi_store_org_ccm_clk_root[IMX8MP_ECSPI_MAX_DEVICES];
static uint32_t ecspi_store_cur_ccm_clk_state[IMX8MP_ECSPI_MAX_DEVICES];
static uint32_t ecspi_store_cur_ccm_clk_root[IMX8MP_ECSPI_MAX_DEVICES];
static uint32_t ecspi_store_cur_ctrl_reg_val[IMX8MP_ECSPI_MAX_DEVICES];

static inline uint32_t read_reg_ecspi(void *ecspi_base, uint32_t reg_addr)
{
	uint32_t read_result;

	read_result = *(volatile uint32_t *)(ecspi_base + reg_addr);

	return read_result;
}

static inline uint32_t write_reg_ecspi(void *ecspi_base, uint32_t reg_addr, uint32_t wr_val)
{
	*(volatile uint32_t *)(ecspi_base + reg_addr) = wr_val;

	return 0;
}


inline int ecspi_rx_available(void *ecspi_base)
{
	uint32_t read_result;

	read_result = (*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT)) & IMX8MP_ECSPI_STAT_RR;

	return read_result;
}


static inline void ecspi_disable(void *ecspi_base)
{
	uint32_t ctrl;

	ctrl = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL);
	ctrl &= ~IMX8MP_ECSPI_CTRL_ENABLE;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = ctrl;
}

inline void ecspi_reset_fifo(void *ecspi_base)
{
	uint32_t read_result;

	/* drain receive buffer */
	while (ecspi_rx_available(ecspi_base))
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_CSPIRXDATA);
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
	if (read_result & IMX8MP_ECSPI_STAT_TC) {
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
		*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT) = read_result;
		pr_debug("WARNING: Clear TX FIFO  FULL \r\n");
	}
	if (read_result & IMX8MP_ECSPI_STAT_RO) {
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
		*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT) = read_result;
		pr_debug("WARNING: Clear RXFIFO has overflowed \r\n");
	}
}

static void  ecspi_dump_regs(void *ecspi_base)
{

#ifdef ENABLE_ESPI_DEBUG
	uint32_t read_result;

	pr_info("Dump Regs..\r\n");

	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_CTRL);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_CTRL, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_CONFIG);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_CONFIG, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_INT);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_INT, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_DMA);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_DMA, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_STAT);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_STAT, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_PERIODREG);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_PERIODREG, read_result);
	read_result = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_TESTREG);
	pr_debug("RegAddr 0x%08X ReadVal 0x%X\r\n", IMX8MP_ECSPI_TESTREG, read_result);

	/*Additional debug info if needed */
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
	if (read_result & IMX8MP_ECSPI_STAT_TC)
		pr_debug("===>Transfer completed\r\n");
	else if (read_result & IMX8MP_ECSPI_STAT_RO)
		pr_debug("===>RXFIFO has overflowed==\r\n");
	else if (read_result & IMX8MP_ECSPI_STAT_RF)
		pr_debug("===>RXFIFO Full==\r\n");
	else if (read_result & IMX8MP_ECSPI_STAT_RDR)
		pr_debug("===>Number of data entries in the RXFIFO is greater than RX_THRESHOLD\r\n");
	else if (read_result & IMX8MP_ECSPI_STAT_RR)
		pr_debug("===>More than 1 word in RXFIFO\r\n");
	else if (read_result & IMX8MP_ECSPI_STAT_TF)
		pr_debug("===>TXFIFO is Full.==\r\n");
	else if ((read_result & IMX8MP_ECSPI_STAT_TDR) == 0)
		pr_debug("===>Number of valid data slots in TXFIFO is greater than TX_THRESHOLD==\r\n");
	else if ((read_result & IMX8MP_ECSPI_STAT_TE) == 0)
		pr_debug("===>TXFIFO contains one or more words==\r\n");
#endif
}

static int32_t config_ecspi_iomux_setting(uint32_t ecspi_chan, uint32_t enable)
{
	void *iomux_base;
	uint32_t reg_offset, read_reg;

	iomux_base = mmap(NULL,
		IMX8MP_IOMUX_CTRL_CCSR_SIZE,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		ecspi_fd[ecspi_chan],
		IMX8MP_IOMUX_CTRL_PHY_ADDR);
	if (iomux_base == (void *) -1) {
		pr_info("Fail to mmap iomux ctrl access \r\n");
		return -1;
	}

	switch (ecspi_chan) {
	case IMX8MP_ECSPI_1:
		reg_offset = IMX8MP_SW_MUX_CTL_PAD_ECSPI1_SS0_OFFSET;
		break;
	case IMX8MP_ECSPI_2:
		reg_offset = IMX8MP_SW_MUX_CTL_PAD_ECSPI2_SS0_OFFSET;
		break;
	case IMX8MP_ECSPI_3:
		reg_offset = IMX8MP_SW_MUX_CTL_PAD_ECSPI3_SS0_OFFSET;
		break;
	default:
		reg_offset = IMX8MP_SW_MUX_CTL_PAD_ECSPI1_SS0_OFFSET;
		break;
	}

	if (enable) {
		ecspi_iomux_state[ecspi_chan] = *(volatile uint32_t *)(iomux_base + reg_offset);
		if (ecspi_chan == IMX8MP_ECSPI_3)
			*(volatile uint32_t *)(iomux_base + reg_offset) = (ecspi_iomux_state[ecspi_chan] & (~IMX8MP_SW_MUX_CTL_PAD_ECSPI_SS0_MASK)) | 0x1;
		else
			*(volatile uint32_t *)(iomux_base + reg_offset) = ecspi_iomux_state[ecspi_chan] & (~IMX8MP_SW_MUX_CTL_PAD_ECSPI_SS0_MASK);
		pr_debug("ECSPI iomux setting done  successfully status..\r\n");
	} else {
		/* restore iomux ctrl settings */
		read_reg  = *(volatile uint32_t *)(iomux_base + reg_offset);
		read_reg = read_reg & (~IMX8MP_SW_MUX_CTL_PAD_ECSPI_SS0_MASK);
		read_reg = read_reg | (ecspi_iomux_state[ecspi_chan] & IMX8MP_SW_MUX_CTL_PAD_ECSPI_SS0_MASK);
		*(volatile uint32_t *)(iomux_base + reg_offset) = read_reg;
		pr_debug("ECSPI iomux setting restored  successfully..\r\n");

	}
	if (munmap(iomux_base, IMX8MP_IOMUX_CTRL_CCSR_SIZE) == -1) {
		pr_info("unmap memory from user space fail for iomux ctrl..\n");
		return -1;
	}

	return 0;
}


int32_t config_ecspi_clk_setting(uint32_t ecspi_chan, ecspi_clk_t clk, ecspi_state_t  state)
{
	void *ccm_clk_base;
	uint32_t reg_addr_CCM_CCGR, reg_addr_CLK_ROOT, reg_read;

	ccm_clk_base = mmap(NULL,
		IMX8MP_CCM_CLK_CTRL_CCSR_SIZE,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		ecspi_fd[ecspi_chan],
		IMX8MP_CCM_CLK_CTRL_PHY_ADDR);

	if (ccm_clk_base == (void *) -1) {
		pr_info("Fail to mmap ccm clk ctrl access \r\n");
		return -1;
	}

	switch (ecspi_chan) {
	case IMX8MP_ECSPI_1:
		reg_addr_CCM_CCGR = IMX8MP_CCM_CCGR7_CLK_OFFSET;
		reg_addr_CLK_ROOT = IMX8MP_ECSPI1_CLK_ROOT_OFFSET;
		break;
	case IMX8MP_ECSPI_2:
		reg_addr_CCM_CCGR = IMX8MP_CCM_CCGR8_CLK_OFFSET;
		reg_addr_CLK_ROOT = IMX8MP_ECSPI2_CLK_ROOT_OFFSET;
		break;
	case IMX8MP_ECSPI_3:
		reg_addr_CCM_CCGR = IMX8MP_CCM_CCGR9_CLK_OFFSET;
		reg_addr_CLK_ROOT = IMX8MP_ECSPI3_CLK_ROOT_OFFSET;
		break;
	default:
		/*Slot#i is default */
		reg_addr_CCM_CCGR = IMX8MP_CCM_CCGR7_CLK_OFFSET;
		reg_addr_CLK_ROOT = IMX8MP_ECSPI1_CLK_ROOT_OFFSET;
		break;
	}

	switch (state) {
	case ECSPI_ENABLE:
		ecspi_store_org_ccm_clk_state[ecspi_chan] = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR);
		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR) = ecspi_store_org_ccm_clk_state[ecspi_chan] | IMX8MP_CCM_DOMAIN_CLK_ALWAYS;

		ecspi_store_org_ccm_clk_root[ecspi_chan] = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CLK_ROOT);
		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CLK_ROOT) = (1 << 28) | (clk.ecspi_root_clk << 24) | (clk.ecspi_ccm_target_root_pre_podf_div_clk << 16) | (clk.ecspi_ccm_target_root_post_podf_div_clk & 0x3f);
		/* store current value of clock setting for debug and query by application*/
		ecspi_store_cur_ccm_clk_state[ecspi_chan] = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR);
		ecspi_store_cur_ccm_clk_root[ecspi_chan]  = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CLK_ROOT);

		pr_debug("ECSPI clock enabled successfully..\r\n");
		break;
	case ECSPI_DISABLE:
		reg_read = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR);
		reg_read = reg_read & (~IMX8MP_CCM_DOMAIN_CLK_ALWAYS);
		reg_read = reg_read | (ecspi_store_org_ccm_clk_state[ecspi_chan] & IMX8MP_CCM_DOMAIN_CLK_ALWAYS);
		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR) = reg_read;

		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CLK_ROOT) = ecspi_store_org_ccm_clk_root[ecspi_chan];
		pr_debug("ECSPI clock setting restored  successfully..\r\n");
		break;
	case ECSPI_SUSPEND:
		reg_read = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR);
		reg_read = reg_read & (~IMX8MP_CCM_DOMAIN_CLK_ALWAYS);
		reg_read = reg_read | (ecspi_store_org_ccm_clk_state[ecspi_chan] & IMX8MP_CCM_DOMAIN_CLK_ALWAYS);
		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR) = reg_read;
		pr_debug("ECSPI clock suspended successfully..\r\n");
		break;
	case ECSPI_RESUME:
		*(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR) = ecspi_store_org_ccm_clk_state[ecspi_chan] | IMX8MP_CCM_DOMAIN_CLK_ALWAYS;
		pr_debug("ECSPI clock resumed successfully..\r\n");
		break;
	}

#ifdef ENABLE_ESPI_DEBUG
		reg_read = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CCM_CCGR);
		pr_debug("Clk IMX8MP_CCM_CCGR7_CLK_OFFSET Reg 0x%X Val 0x%X\r\n", reg_addr_CCM_CCGR, reg_read);
		reg_read = *(volatile uint32_t *)(ccm_clk_base + reg_addr_CLK_ROOT);
		pr_debug("Clk IMX8MP_ECSPI_CLK_ROOT_OFFSET Reg 0x%X Val 0x%X\r\n", reg_addr_CLK_ROOT, reg_read);
#endif
	if (munmap(ccm_clk_base, IMX8MP_CCM_CLK_CTRL_CCSR_SIZE) == -1) {
		pr_info("unmap memory from user space fail for ccm clk ctrl..\n");
		return -1;
	}

	return 0;
}

int32_t imx_spi_clk_suspend_resume(uint32_t ecspi_chan, ecspi_state_t  state)
{
	int32_t ret = -1;
	ecspi_clk_t clk; //dummy clock to make function happy

	if ((state == ECSPI_SUSPEND) || (state == ECSPI_RESUME))
		ret = config_ecspi_clk_setting(ecspi_chan, clk, state);
	else
		pr_info("Invalid state %d \r\n", state);
	return ret;
}


static void ecspi_int(void *ecspi_base, uint16_t ecspi_chan, ecspi_clk_t clk)
{
	uint32_t testreg;
	uint32_t ctrl_reg_val = 0;
	uint32_t config_reg_val = 0;
	uint32_t period_reg_val = 0;

	/*Disable Power Management  and configure clock for ECSPI */
	config_ecspi_clk_setting(ecspi_chan, clk, ECSPI_ENABLE);

	/*configure Pin mux setting for ECSPI */
	config_ecspi_iomux_setting(ecspi_chan, ECSPI_ENABLE);

	/*Disable ECSPI before re-enable to reset ecspi to default*/
	ecspi_disable(ecspi_base);

	/*
	 * The ctrl register must be written first, with the EN bit set other
	 * registers must not be written to.
	 */
	ctrl_reg_val = (0x01F << IMX8MP_ECSPI_CTRL_BL_OFFSET) | IMX8MP_ECSPI_CTRL_CS(0) | \
		       IMX8MP_ECSPI_CTRL_DRCTL(0) | (clk.ecspi_ctrl_pre_div_clk << IMX8MP_ECSPI_CTRL_PREDIV_OFFSET)| \
		       (clk.ecspi_ctrl_post_div_clk << IMX8MP_ECSPI_CTRL_POSTDIV_OFFSET) | \
		       IMX8MP_ECSPI_CTRL_MODE_MASK | IMX8MP_ECSPI_CTRL_ENABLE;
	config_reg_val = IMX8MP_ECSPI_CONFIG_SBBCTRL(0);
	period_reg_val = IMX8MP_ECSPI_PERIODREG_CSDCTL(1);
	write_reg_ecspi(ecspi_base, IMX8MP_ECSPI_CTRL, ctrl_reg_val);
	write_reg_ecspi(ecspi_base, IMX8MP_ECSPI_CONFIG, config_reg_val);
	write_reg_ecspi(ecspi_base, IMX8MP_ECSPI_PERIODREG, period_reg_val);

	testreg = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_TESTREG);
#ifdef SPI_LOOP_TEST_ENABLE
	testreg |= IMX8MP_ECSPI_TESTREG_LBC;
	pr_debug("Loopback Mode Enabled Reg 0x%X 0x%x ==\r\n", IMX8MP_ECSPI_TESTREG, testreg);
#else
	testreg &= ~IMX8MP_ECSPI_TESTREG_LBC;
#endif
	write_reg_ecspi(ecspi_base, IMX8MP_ECSPI_TESTREG, testreg);
	/*Throw if any data in RX FIFO */
	ecspi_reset_fifo(ecspi_base);
	/*Store current ctrl reg value for debug and query by application if needed*/
	ecspi_store_cur_ctrl_reg_val[ecspi_chan] = read_reg_ecspi(ecspi_base, IMX8MP_ECSPI_CTRL);

	/*Dump Regs*/
	ecspi_dump_regs(ecspi_base);
}

static inline void ecspi_set_brust_size(void *ecspi_base, uint16_t brust_size)
{
	uint32_t ctrl, store_ctrl;
	uint32_t ctrl_reg_val = 0;

	ctrl = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL);
	store_ctrl = ctrl;
	ctrl &= ~IMX8MP_ECSPI_CTRL_ENABLE;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = ctrl;

	store_ctrl &=  0x000FFFFF;
	store_ctrl |= (brust_size << IMX8MP_ECSPI_CTRL_BL_OFFSET) | IMX8MP_ECSPI_CTRL_ENABLE;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = store_ctrl;
}

int32_t imx_spi_rx(void *ecspi_base, uint16_t ecspi_chan, uint16_t addr_val, uint16_t *val_p)
{
	uint32_t read_result;
	uint32_t tranfer_val;
#ifdef ECSPI_ERROR_CHECK_ENABLE
	if (ecspi_chan >= IMX8MP_ECSPI_MAX_DEVICES) {
		pr_info("Invalid ecspi chan %d \r\n", ecspi_chan);
		return 1;
	}
	ecspi_base = ecspi_handle[ecspi_chan];
	if (ecspi_handle[ecspi_chan] == NULL) {
		pr_info("ecspi device not initialized..\r\n");
		return 1;
	}
#endif
	tranfer_val =  (uint32_t) (addr_val << 16);
	*(volatile uint32_t *)(ecspi_base + IMX8MP_CSPITXDATA) = tranfer_val;

	/* Set XCH*/
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL);
	read_result |= IMX8MP_ECSPI_CTRL_XCH;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = read_result;
	while (1) {
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
		if (read_result & IMX8MP_ECSPI_STAT_TC)
			break;
	}
	/* Check  RXFIFO for data*/
	while (1) {
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
		if (read_result & IMX8MP_ECSPI_STAT_RR) {
			/* ReadBack received data*/
			*val_p  = *(volatile uint16_t *)(ecspi_base + IMX8MP_CSPIRXDATA);
			if (!ecspi_rx_available(ecspi_base))
				break;
		}
	}
	//clear Transfer Completed Status and RXFIFO Overflow.
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT) = read_result;

	return 0;
}

int32_t diora_phal_read16(void *ecspi_base, uint16_t ecspi_chan, uint16_t addr_val, uint16_t *val_p)
{
	uint32_t read_result;
	uint16_t tranfer_val, read_val;
	uint16_t ctr;
#ifdef ECSPI_ERROR_CHECK_ENABLE
	if (ecspi_chan >= IMX8MP_ECSPI_MAX_DEVICES) {
		pr_info("Invalid ecspi chan %d \r\n", ecspi_chan);
		return 1;
	}
	ecspi_base = ecspi_handle[ecspi_chan];
	if (ecspi_handle[ecspi_chan] == NULL) {
		pr_info("ecspi device not initialized..\r\n");
		return 1;
	}
#endif
	ecspi_set_brust_size(ecspi_base, BURST_SIZE_16);

	for (ctr = 0; ctr < 2; ctr++) {
		if (!ctr) {
			*(volatile uint16_t *)(ecspi_base + IMX8MP_CSPITXDATA) = addr_val;
		} else {
			/*Make sure to clean Rx FIFO */
			ecspi_reset_fifo(ecspi_base);
			/*Send dummy write */
			*(volatile uint16_t *)(ecspi_base + IMX8MP_CSPITXDATA) = 0x00;
		}
		/*Set XCH*/
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL);
		read_result |= IMX8MP_ECSPI_CTRL_XCH;
		 *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = read_result;
		while (1) {
			read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
			if (read_result & IMX8MP_ECSPI_STAT_TC)
				break;
		}
	}
	/*Check  RXFIFO for data*/
	while (1) {
		if (ecspi_rx_available(ecspi_base)) {
			/*ReadBack received data*/
			*val_p = *(volatile uint16_t *)(ecspi_base + IMX8MP_CSPIRXDATA);
		} else
			break;
	}
	//clear Transfer Completed Status and RXFIFO Overflow.
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT) = read_result;

	/*Restore frame size 32 bit*/
	ecspi_set_brust_size(ecspi_base, BURST_SIZE_32);
	return 0;
}

int32_t imx_spi_tx(void *ecspi_base, uint16_t ecspi_chan, uint16_t addr_val, uint16_t wr_val)
{
	uint32_t read_result;
	uint32_t tranfer_val;

#ifdef ECSPI_ERROR_CHECK_ENABLE
	if (ecspi_chan >= IMX8MP_ECSPI_MAX_DEVICES) {
		pr_info("Invalid ecspi chan %d \r\n", ecspi_chan);
		return 1;
	}
	ecspi_base = ecspi_handle[ecspi_chan];

	if (ecspi_handle[ecspi_chan] == NULL) {
		pr_info("ecspi device not initialized..\r\n");
		return 1;
	}
#endif
	tranfer_val =  (1 << 31) | (addr_val << 16) | wr_val;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_CSPITXDATA) = tranfer_val;

	/*Set XCH to enable transfer*/
	read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL);
	read_result |= IMX8MP_ECSPI_CTRL_XCH;
	*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_CTRL) = read_result;
	/*Read Status Reg*/
	while (1) {
		read_result = *(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT);
		if (read_result & IMX8MP_ECSPI_STAT_TC) {
			*(volatile uint32_t *)(ecspi_base + IMX8MP_ECSPI_STAT) = read_result;
			break;
		}
	}

	ecspi_reset_fifo(ecspi_base);

	return 0;
}

void *imx_spi_init_with_clk(uint32_t ecspi_chan, ecspi_clk_t clk)
{
	void *ecspi_base;

	if (ecspi_chan >= IMX8MP_ECSPI_MAX_DEVICES) {
		pr_info("Invalid ecspi device..\r\n");
		return NULL;
	}

	if (ecspi_handle[ecspi_chan] == NULL) {
		ecspi_fd[ecspi_chan] = open("/dev/mem", O_RDWR | O_SYNC);
		if (ecspi_fd[ecspi_chan] < 0) {
			pr_info("Fail to open dev mem file descriptor..\r\n");
			return NULL;
		}
		ecspi_base = mmap(NULL,
				IMX8MP_ECSPI_CCSR_SIZE,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				ecspi_fd[ecspi_chan],
				IMX8MP_ECSPI_PHY_ADDR);
		if (ecspi_base == (void *) -1) {
			pr_info("Fail to mmap ecspi memory map.. \r\n");
			return NULL;
		}
		pr_info("ECSPI %d successfully memory mapped at address %p\r\n", (ecspi_chan + 1), ecspi_base);

		ecspi_handle[ecspi_chan] = ecspi_base + (ecspi_chan * IMX8MP_ECSPI_CHANNEL_OFFSET);
		ecspi_int(ecspi_handle[ecspi_chan], ecspi_chan, clk);
	}
	return ecspi_handle[ecspi_chan];
}

void *imx_spi_init(uint32_t ecspi_chan, uint32_t clock_frequency)
{
	ecspi_clk_t clk;
	switch (clock_frequency) {
	case 1:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_24M_REF_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x17; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x0; /*   Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x0; /*  Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("1 MHz Clock  Selected\r\n");
		break;
	case 4:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_24M_REF_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x5; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x0; /*   Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x0; /*   Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("4 MHz Clock  Selected\r\n");
		break;
	case 8:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_24M_REF_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x2; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x0; /*   Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x0; /*   Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("8 MHz Clock  Selected\r\n");
		break;
	case 16:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_SYSTEM_PLL1_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x0; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x5; /*   Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x3; /*  Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("16 MHz Clock  Selected\r\n");
		break;
	case 24:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_24M_REF_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x0; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x0; /*  0x5 Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x0; /*  0x3 Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("24 MHz Clock  Selected\r\n");
		break;
	default:
		clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_SYSTEM_PLL1_CLK;
		clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /*  Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
		clk.ecspi_ccm_target_root_post_podf_div_clk = 0x0; /*  Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
		clk.ecspi_ctrl_pre_div_clk = 0x5; /*  0x5 Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
		clk.ecspi_ctrl_post_div_clk = 0x3; /*  0x3 Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
		printf("Setting default 16 MHZ clock\r\n");
		break;
	}

	return imx_spi_init_with_clk(ecspi_chan, clk);
}

void imx_get_spi_clk_config(void *ecspi_base, uint16_t ecspi_chan, ecspi_clk_t *clk)
{
	clk->ecspi_root_clk = (ecspi_store_cur_ccm_clk_root[ecspi_chan] >>  24) & 0x7;
	clk->ecspi_ccm_target_root_pre_podf_div_clk = (ecspi_store_cur_ccm_clk_root[ecspi_chan] >>  16) & 0x7;;
	clk->ecspi_ccm_target_root_post_podf_div_clk = ecspi_store_cur_ccm_clk_root[ecspi_chan]  & 0x3f;;
	clk->ecspi_ctrl_pre_div_clk = (ecspi_store_cur_ctrl_reg_val[ecspi_chan] >> 12) & 0xf;
	clk->ecspi_ctrl_post_div_clk = (ecspi_store_cur_ctrl_reg_val[ecspi_chan] >> 8) & 0xf;
}

int32_t  imx_spi_deinit(uint32_t ecspi_chan)
{
	ecspi_clk_t clk;

	if (ecspi_chan >= IMX8MP_ECSPI_MAX_DEVICES) {
		pr_info("Invalid ecspi device..\r\n");
		return -1;
	}

	if (ecspi_handle[ecspi_chan] == NULL) {
		pr_info("ecspi device not initialized..\r\n");
		return -1;
	}
	config_ecspi_iomux_setting(ecspi_chan, ECSPI_DISABLE);
	config_ecspi_clk_setting(ecspi_chan, clk, ECSPI_DISABLE);
	if (munmap(ecspi_handle[ecspi_chan], IMX8MP_ECSPI_CCSR_SIZE) == -1) {
		pr_info("unmap memory from user space fail for ecspi..\n");
		return -1;
	}
	ecspi_handle[ecspi_chan] = NULL;
	close(ecspi_fd[ecspi_chan]);

	return 0;
}
