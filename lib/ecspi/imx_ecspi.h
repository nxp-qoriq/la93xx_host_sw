/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2024 NXP
 */
#ifndef __IMX_ECSPI_H__
#define __IMX_ECSPI_H__

#include <imx_ecspi_api.h>

#define IMX8MP_CSPIRXDATA	0x00
#define IMX8MP_CSPITXDATA	0x04

#define BIT(nr)			(1UL << (nr))
/* generic defines to abstract from the different register layouts */
#define IMX8MP_INT_RR	(1 << 0) /* Receive data ready interrupt */
#define IMX8MP_INT_TE	(1 << 1) /* Transmit FIFO empty interrupt */
#define IMX8MP_INT_RDR	BIT(4) /* Receive date threshold interrupt */

/* The maximum bytes that a sdma BD can transfer. */
#define MAX_SDMA_BD_BYTES (1 << 15)
#define IMX8MP_ECSPI_CTRL_MAX_BURST	512
/* The maximum bytes that IMX53_ECSPI can transfer in slave mode.*/
#define MX53_MAX_TRANSFER_BYTES		512


#define IMX8MP_ECSPI_CTRL		0x08
#define IMX8MP_ECSPI_CTRL_ENABLE		(1 <<  0)
#define IMX8MP_ECSPI_CTRL_XCH		(1 <<  2)
#define IMX8MP_ECSPI_CTRL_SMC		(1 << 3)
#define IMX8MP_ECSPI_CTRL_MODE_MASK	(0xf << 4)
#define IMX8MP_ECSPI_CTRL_DRCTL(drctl)	((drctl) << 16)
#define IMX8MP_ECSPI_CTRL_POSTDIV_OFFSET	8
#define IMX8MP_ECSPI_CTRL_PREDIV_OFFSET	12
#define IMX8MP_ECSPI_CTRL_CS(cs)		((cs) << 18)
#define IMX8MP_ECSPI_CTRL_BL_OFFSET	20
#define IMX8MP_ECSPI_CTRL_BL_MASK		(0xfff << 20)

#define IMX8MP_ECSPI_CONFIG	0x0c
#define IMX8MP_ECSPI_CONFIG_SCLKPHA(cs)	(1 << ((cs) +  0))
#define IMX8MP_ECSPI_CONFIG_SCLKPOL(cs)	(1 << ((cs) +  4))
#define IMX8MP_ECSPI_CONFIG_SBBCTRL(cs)	(1 << ((cs) +  8))
#define IMX8MP_ECSPI_CONFIG_SSBPOL(cs)	(1 << ((cs) + 12))
#define IMX8MP_ECSPI_CONFIG_SCLKCTL(cs)	(1 << ((cs) + 20))

#define IMX8MP_ECSPI_INT		0x10
#define IMX8MP_ECSPI_INT_TEEN		(1 <<  0)
#define IMX8MP_ECSPI_INT_RREN		(1 <<  3)
#define IMX8MP_ECSPI_INT_RDREN		(1 <<  4)

#define IMX8MP_ECSPI_DMA		0x14
#define IMX8MP_ECSPI_DMA_TX_WML(wml)	((wml) & 0x3f)
#define IMX8MP_ECSPI_DMA_RX_WML(wml)	(((wml) & 0x3f) << 16)
#define IMX8MP_ECSPI_DMA_RXT_WML(wml)	(((wml) & 0x3f) << 24)

#define IMX8MP_ECSPI_DMA_TEDEN		(1 << 7)
#define IMX8MP_ECSPI_DMA_RXDEN		(1 << 23)
#define IMX8MP_ECSPI_DMA_RXTDEN		(1 << 31)

#define IMX8MP_ECSPI_STAT		0x18
#define IMX8MP_ECSPI_STAT_TE            (1 <<  0)
#define IMX8MP_ECSPI_STAT_TDR           (1 <<  1)
#define IMX8MP_ECSPI_STAT_TF            (1 <<  2)
#define IMX8MP_ECSPI_STAT_RR            (1 <<  3)
#define IMX8MP_ECSPI_STAT_RDR           (1 <<  4)
#define IMX8MP_ECSPI_STAT_RF            (1 <<  5)
#define IMX8MP_ECSPI_STAT_RO            (1 <<  6)
#define IMX8MP_ECSPI_STAT_TC            (1 <<  7)

#define IMX8MP_ECSPI_PERIODREG   0x1C
#define IMX8MP_ECSPI_PERIODREG_SAMPLEPERIOD(period)      ((period) & 0x7FFF)
#define IMX8MP_ECSPI_PERIODREG_CSRC                      BIT(15)
#define IMX8MP_ECSPI_PERIODREG_CSDCTL(csd)               (((csd) & 0x3f) << 16)

#define IMX8MP_ECSPI_TESTREG	0x20
#define IMX8MP_ECSPI_TESTREG_LBC	BIT(31)

//#define SPI_LOOP_TEST_ENABLE 1
#define IMX8MP_ECSPI_PHY_ADDR  0x30820000
#define IMX8MP_ECSPI_CCSR_SIZE 0x10000UL
#define IMX8MP_IOMUX_CTRL_PHY_ADDR  0x30330000
#define IMX8MP_IOMUX_CTRL_CCSR_SIZE 0x10000UL
#define IMX8MP_SW_MUX_CTL_PAD_ECSPI_SS0_MASK     0x7
#define IMX8MP_SW_MUX_CTL_PAD_ECSPI1_SS0_OFFSET  0x01EC
#define IMX8MP_SW_MUX_CTL_PAD_ECSPI2_SS0_OFFSET  0x01FC
#define IMX8MP_SW_MUX_CTL_PAD_ECSPI3_SS0_OFFSET  0x022C
#define IMX8MP_CCM_CLK_CTRL_PHY_ADDR  0x30380000
#define IMX8MP_CCM_CLK_CTRL_CCSR_SIZE 0x10000UL
#define IMX8MP_CCM_CCGR7_CLK_OFFSET 0x4070
#define IMX8MP_CCM_CCGR8_CLK_OFFSET 0x4080
#define IMX8MP_CCM_CCGR9_CLK_OFFSET 0x4090
#define IMX8MP_CCM_DOMAIN_CLK_ALWAYS 0x3
#define IMX8MP_ECSPI1_CLK_ROOT_OFFSET 0xB280
#define IMX8MP_ECSPI2_CLK_ROOT_OFFSET 0xB300
#define IMX8MP_ECSPI3_CLK_ROOT_OFFSET 0xC180

#endif /* __IMX_ECSPI_H__ */
