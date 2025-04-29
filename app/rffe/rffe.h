/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2025 NXP
 */

#ifndef RFFE_H
#define RFFE_H

#include "HAL-imx8mp.h"

// With the default timer settings, runs at around 35 kHz on L433
#define RFFE_DEFAULT_PRESCALER 56
#define RFFE_DEFAULT_CLKDIV 1

// RFFE standard 32Khz to 26Mhz i.e. period 36µs to 38ns
// gpio takes ~330ns, per bit need 2x clk edge + 1x sda ~ 1µs
// so max freq possible is 1Mhz on i.mx8mp @1.6Ghz
// one write takes ~26µs (SCC + 22bits)
#define RFFE_DEFAULT_PERIOD 1000


// Set the timer to use for driving the pins
#define RFFE_TIMER_ENABLE() {}
#define RFFE_TIM TIM2

// RFFE-specific defines
#define RFFE_READ_FLAG 0b011
#define RFFE_WRITE_FLAG 0b010
#define RFFE_START_SEQ_LEN 2
#define RFFE_SLAVE_ADDR_LEN 4
#define RFFE_READ_WRITE_FLAG_LEN 3
#define RFFE_REG_ADDR_PLUS_PARITY_LEN 6

#define RFFE_EXT_READ_FLAG  0b0010
#define RFFE_EXT_WRITE_FLAG 0b0000
#define RFFE_EXT_READ_WRITE_FLAG_LEN 4
#define RFFE_EXT_REG_ADDR_LEN 8
#define RFFE_EXT_REG_BYTE_COUNT_PLUS_PARITY_LEN 5


typedef struct rffe_s {
	uint8_t slave_addr;
	uint32_t sck;
	uint32_t sda;
	uint8_t port;
	uint16_t tim_prescaler;
	uint16_t tim_period;
} rffe_t;

void RFFE_Init(uint8_t slave_addr, uint8_t dgb_id, uint16_t sck_pin, uint16_t sda_pin);
void RFFE_SetSpeed(uint16_t prescaler, uint16_t period, uint32_t clk_div);
void RFFE_WriteByte(uint8_t addr, uint8_t data);
void RFFE_WriteReg0Byte(uint8_t data);
uint8_t RFFE_ReadByte(uint8_t addr);
uint8_t RFFE_ExtReadByte(uint8_t byteCount, uint8_t addr, uint8_t *outputVal);
uint8_t RFFE_ExtWriteByte(uint8_t byteCount, uint8_t addr, uint8_t *DataVal);
#endif
