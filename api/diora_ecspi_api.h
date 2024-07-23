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


void   *diora_phal_rw_init(u32 ecspi_chan);
int32_t diora_phal_rw_deinit(u32 ecspi_chan);
int32_t diora_phal_read16(void *handle, u16 ecspi_chan, u16  addr, u16 *val_p);
int32_t diora_phal_write16(void *handle, u16 ecspi_chan, u16  addr, u16 val);

#endif /* __DIORA_ECSPI_API_H__ */
