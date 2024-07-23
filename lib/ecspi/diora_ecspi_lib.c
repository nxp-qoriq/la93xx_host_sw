// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2024 NXP

#include "diora_ecspi_api.h"
#include "imx_ecspi_api.h"

int32_t diora_phal_read16(void *handle, u16 ecspi_chan, u16  addr, u16 *val_p)
{
	return imx_spi_rx(handle, ecspi_chan, addr, val_p);
}

int32_t diora_phal_write16(void *handle, u16 ecspi_chan, u16  addr, u16 val)
{
	return imx_spi_tx(handle, ecspi_chan, addr, val);
}

void   *diora_phal_rw_init(u32 ecspi_chan)
{
	return imx_spi_init(ecspi_chan);
}

int32_t diora_phal_rw_deinit(u32 ecspi_chan)
{
	return imx_spi_deinit(ecspi_chan);
}
