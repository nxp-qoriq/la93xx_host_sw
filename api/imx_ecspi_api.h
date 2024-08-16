/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __IMX_ECSPI_API_H__
#define __IMX_ECSPI_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>

//#define ENABLE_ESPI_DEBUG 1
#if ENABLE_ESPI_DEBUG
#define pr_debug(...) printf(__VA_ARGS__)
#else
#define pr_debug(...)
#endif
#define pr_info(...) printf(__VA_ARGS__)
#define IMX_ECSPI_SUCCESS	0
#define IMX_ECSPI_FAIL	-1

/*Enable to check_user supplied pointer and channel in data path */
//#define ECSPI_ERROR_CHECK_ENABLE

#define u16 uint16_t
#define u32 uint32_t

enum ecspi_imx_dev {
	IMX8MP_ECSPI_1, /*slot#1*/
	IMX8MP_ECSPI_2,/*slot#2*/
	IMX8MP_ECSPI_3,/*slot#3*/
	IMX8MP_ECSPI_MAX_DEVICES,
};

typedef enum ecspi_state {
	ECSPI_DISABLE,
	ECSPI_ENABLE,
	ECSPI_SUSPEND,
	ECSPI_RESUME,
} ecspi_state_t;

typedef struct ecspi_clk {
	u32 ecspi_root_clk;
	u16 ecspi_ccm_target_root_pre_podf_div_clk;
	u16 ecspi_ccm_target_root_post_podf_div_clk;
	u16 ecspi_ctrl_pre_div_clk;
	u16 ecspi_ctrl_post_div_clk;
} ecspi_clk_t;

typedef enum ecspi_imx_clk {
	IMX8MP_ECSPI_CLK_24M_REF_CLK = 0, /*b000, 24M_REF_CLK*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL1_CLK = 4, /*b100, SYSTEM_PLL1_CLK*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL1_DIV5 = 3, /*b011 SYSTEM_PLL1_DIV5*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL1_DIV20 = 2, /*b010, SYSTEM_PLL1_DIV20*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL2_DIV4 = 6, /*b110, SYSTEM_PLL2_DIV4*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL2_DIV5 = 1,/*b001, SYSTEM_PLL2_DIV5*/
	IMX8MP_ECSPI_CLK_SYSTEM_PLL3_CLK = 5,/*b101 SYSTEM_PLL3_CLK*/
	IMX8MP_ECSPI_CLK_AUDIO_PLL2_CLK = 7,/*b111 AUDIO_PLL2_CLK*/
} ecspi_imx_clk_t;

int32_t imx_spi_rx(void *handle, u16 ecspi_chan, u16  addr, u16 *val_p);
int32_t imx_spi_tx(void *handle, u16 ecspi_chan, u16  addr, u16 val);
void   *imx_spi_init(u32 ecspi_chan, u32 clock_frequency);
void *imx_spi_init_with_clk(uint32_t ecspi_chan, ecspi_clk_t clk);
int32_t imx_spi_deinit(u32 ecspi_chan);
int32_t imx_spi_clk_suspend_resume(uint32_t ecspi_chan, ecspi_state_t  state);
void imx_get_spi_clk_config(void *ecspi_base, uint16_t ecspi_chan, ecspi_clk_t *clk);
#endif /* __IMX_ECSPI_API_H__ */
