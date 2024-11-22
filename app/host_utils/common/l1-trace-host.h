/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2024 NXP
 */

#ifndef __L1_TRACE_HOST_H__
#define __L1_TRACE_HOST_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define L1_TRACE 1
#define L1_TRACE_HOST_SIZE 100
#define L1_TRACE_M7_SIZE 100

typedef struct l1_trace_host_data_s {
    uint64_t cnt;
    uint32_t msg;
    uint32_t param;
} l1_trace_host_data_t;

typedef struct l1_trace_host_code_s {
    uint32_t msg;
    char text[100];
} l1_trace_host_code_t;

void l1_trace(uint32_t msg, uint32_t param);
void l1_trace_clear(void);

extern l1_trace_host_data_t l1_trace_host_data[];

enum l1_trace_msg_host {
    /* 0x100 */ L1_TRACE_MSG_DMA_XFR = 0x100,
    /* 0x101 */ L1_TRACE_MSG_DMA_CFGERR,
    /* 0x102 */ L1_TRACE_MSG_DMA_DDR_RD_START,
    /* 0x103 */ L1_TRACE_MSG_DMA_DDR_RD_COMP,
    /* 0x104 */ L1_TRACE_MSG_DMA_DDR_RD_UNDERRUN,
    /* 0x105 */ L1_TRACE_MSG_DMA_DDR_WR_START,
    /* 0x106 */ L1_TRACE_MSG_DMA_DDR_WR_COMP,
    /* 0x107 */ L1_TRACE_MSG_DMA_DDR_WR_OVERRUN,
};

#endif
