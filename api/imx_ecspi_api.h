/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __IMX_ECSPI_API_H__
#define __IMX_ECSPI_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>

#if ENABLE_ESPI_DEBUG
#define pr_debug(...) printf(__VA_ARGS__)
#else
#define pr_debug(...)
#endif
#define pr_info(...) printf(__VA_ARGS__)
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

int32_t spi_imx_rx(void *handle, u16 ecspi_chan, u16  addr, u16 *val_p);
int32_t spi_imx_tx(void *handle, u16 ecspi_chan, u16  addr, u16 val);
void   *imx_spi_init(u32 ecspi_chan);
int32_t imx_spi_deinit(u32 ecspi_chan);

#endif /* __IMX_ECSPI_API_H__ */
