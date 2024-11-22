/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "imx8-host.h"

void invalidate_region(void* region, uint32_t size)
{
 uint32_t *ptr;
 uint32_t index;

 for (ptr = (uint32_t*)region, index = 0; index < size/CACHE_LINE_SIZE; ptr += 16, index++) {
	dccivac((uint32_t *)(ptr));
	}
}

void flush_region(void *region, uint32_t size)
{
 uint32_t *ptr;
 uint32_t index;

 for (ptr = (uint32_t*)region, index = 0; index < size/CACHE_LINE_SIZE; ptr += 16, index++) {
	dcbf((uint32_t *)(ptr));
	}
}
