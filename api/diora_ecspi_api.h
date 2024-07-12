/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2024 NXP
 */

#ifndef __DIORA_ECSPI_API_H__
#define __DIORA_ECSPI_API_H__

#include <stdint.h>
#include <stdlib.h>
#include <linux/types.h>
#include "imx_ecspi_api.h"

typedef int32_t     error_t;
typedef uint32_t    u32;
typedef uint16_t    u16;

#define diora_phal_read16  spi_imx_rx
#define diora_phal_write16 spi_imx_tx
#define diora_phal_rw_init imx_spi_init
#define diora_phal_rw_deinit imx_spi_deinit

#endif /* __DIORA_ECSPI_API_H__ */
