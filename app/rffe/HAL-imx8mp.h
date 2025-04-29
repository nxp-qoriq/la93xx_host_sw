/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2025 NXP
 */

#ifndef INCLUDE_HAL_IMX8MP_H_
#define INCLUDE_HAL_IMX8MP_H_

#define GPIO_BANK 0
#define GPIO_NUM 8

//SKY58281-11
//MIPI_RFFE_RX_CLK    AE16  SAI5_RXD0 AE16   GPIO3_IO21
//MIPI_RFFE_RX_DATA   AD16  SAI5_RXD1 AD16   GPIO3_IO22
//MIPI_RFFE_TX_CLK    AF16  SAI5_RXD2 AF16   GPIO3_IO23
//MIPI_RFFE_TX_DATA   AE14  SAI5_RXD3 AE14   GPIO3_IO24

#define RFNM_GPIO3_21 ((3 << GPIO_BANK) | (21 << GPIO_NUM))
#define RFNM_GPIO3_22 ((3 << GPIO_BANK) | (22 << GPIO_NUM))
#define RFNM_GPIO3_23 ((3 << GPIO_BANK) | (23 << GPIO_NUM))
#define RFNM_GPIO3_24 ((3 << GPIO_BANK) | (24 << GPIO_NUM))
#define RFNM_GPIO5_00 ((5 << GPIO_BANK) | (00 << GPIO_NUM))
#define RFNM_GPIO5_10 ((5 << GPIO_BANK) | (10 << GPIO_NUM))


typedef int GPIO_TypeDef;

int HAL_Init(void);

int HAL_Delay(int delay); // Give the slave a few milliseconds to get out of write mode...

int HAL_TIM_Base_Init(void);
int HAL_TIM_Base_Start(void);
int HAL_TIM_Base_Stop(void);
int __HAL_TIM_GET_COUNTER_ns(void);
int __HAL_TIM_GET_COUNTER_us(void);

#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET   1

int HAL_GPIO_Init(void);
void SetPinOutput(GPIO_TypeDef port, uint16_t pin);
void SetPinInput(GPIO_TypeDef port, uint16_t pin);
int HAL_GPIO_WritePin(GPIO_TypeDef port, uint16_t pin, int value);
int HAL_GPIO_WritePin(GPIO_TypeDef port, uint16_t pin, int value);
int HAL_GPIO_ReadPin(GPIO_TypeDef port, uint16_t pin);

#define L1_TRACE 1
#define L1_TRACE_HOST_SIZE 100

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
    /* 0x100 */ L1_TRACE_MSG_GPIO_WRITE = 0x100,
    /* 0x101 */ L1_TRACE_MSG_GPIO_READ,
};

void print_host_trace(void);
extern volatile uint32_t l1_trace_disable;

#endif
