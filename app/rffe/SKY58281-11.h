/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2025 NXP
 */

#ifndef SKY58281_RFFE_H
#define SKY58281_RFFE_H

/* Definitions parsed from the SKY58281_21 datasheet.
 */

#define SKY58281_TX_SLAVE_ADDR 0b1000
#define SKY58281_RX_SLAVE_ADDR 0b1011

 // common
#define SKY58281_REG_PRODUCT_ID         0x1D // read only  0x04 (TX)  0x06 (RX)
#define SKY58281_REG_MANUFACTURER_ID    0x1E // read only  0xA5
#define SKY58281_REG_MAN_USID           0x1F
#define SKY58281_REG_EXT_PRODUCT_ID     0x20
#define SKY58281_REG_REV_ID             0x21
#define SKY58281_REG_GROUP_SID          0x22
#define SKY58281_REG_UDR_RST            0x23
#define SKY58281_REG_ERR_SUM            0x24
#define SKY58281_REG_BUS_LD             0x2B

// SKY58281-11 TX Register Map
#define SKY58281_TX_REG_PA_CTRL_0          0x00
#define SKY58281_TX_REG_PA_BIAS_0          0x01
#define SKY58281_TX_REG_ASM_CTRL           0x02
#define SKY58281_TX_REG_CPL_CTRL           0x03
#define SKY58281_TX_REG_PA_BIAS_1          0x04
#define SKY58281_TX_REG_PM_TRIG            0x1C
#define SKY58281_TX_REG_PRODUCT_ID         0x1D
#define SKY58281_TX_REG_MANUFACTURER_ID    0x1E
#define SKY58281_TX_REG_MAN_USID           0x1F
#define SKY58281_TX_REG_EXT_PRODUCT_ID     0x20
#define SKY58281_TX_REG_REV_ID             0x21
#define SKY58281_TX_REG_GROUP_SID          0x22
#define SKY58281_TX_REG_UDR_RST            0x23
#define SKY58281_TX_REG_ERR_SUM            0x24
#define SKY58281_TX_REG_BUS_LD             0x2B

// SKY58281-11 RX Register Map
#define SKY58281_RX_REG_LNA1_N77_CONFIG       0x00
#define SKY58281_RX_REG_LNA2_N79_CONFIG       0x01
#define SKY58281_RX_REG_LNA3_N77_CONFIG       0x02
#define SKY58281_RX_REG_LNA4_N79_CONFIG       0x03
#define SKY58281_RX_REG_Gain Mode             0x04
#define SKY58281_RX_REG_SWITCH_LNA12_CONFIG  0x05
#define SKY58281_RX_REG_SWITCH_LNA34_CONFIG  0x06
#define SKY58281_RX_REG_LNA1_N77_GAIN_BIAS    0x08
#define SKY58281_RX_REG_LNA2_N79_GAIN_BIAS    0x09
#define SKY58281_RX_REG_LNA2_N79_GAIN_BIAS    0x09
#define SKY58281_RX_REG_LNA3_N77_GAIN_BIAS    0x0A
#define SKY58281_RX_REG_LNA4_N79_GAIN_BIAS    0x0B
#define SKY58281_RX_REG_UDR_SET_AB CONTROL   0x11
#define SKY58281_RX_REG_UDR_SET_CD CONTROL   0x12
#define SKY58281_RX_REG_PM_TRIG               0x1C
#define SKY58281_RX_REG_PRODUCT_ID            0x1D
#define SKY58281_RX_REG_MANUFACTURER_ID       0x1E
#define SKY58281_RX_REG_MAIN_USID             0x1F
#define SKY58281_RX_REG_EXT_PRODUCT_ID        0x20
#define SKY58281_RX_REG_REV_ID                0x21
#define SKY58281_RX_REG_GROUP_SID             0x22
#define SKY58281_RX_REG_UDR_RST               0x23
#define SKY58281_RX_REG_ERR_SUM               0x24
#define SKY58281_RX_REG_BUS_LD                0x2B
#define SKY58281_RX_REG_EXT_TRIGGER_MASK      0x2D
#define SKY58281_RX_REG_EXT_TRIGGER           0x2E
#define SKY58281_RX_REG_EXT_TRIG_CNT_3        0x38
#define SKY58281_RX_REG_EXT_TRIG_CNT_4        0x39
#define SKY58281_RX_REG_EXT_TRIG_CNT_5        0x3A
#define SKY58281_RX_REG_EXT_TRIG_CNT_6        0x3B
#define SKY58281_RX_REG_EXT_TRIG_CNT_7        0x3C
#define SKY58281_RX_REG_EXT_TRIG_CNT_8        0x3D
#define SKY58281_RX_REG_EXT_TRIG_CNT_9        0x3E
#define SKY58281_RX_REG_EXT_TRIG_CNT_10       0x3F

#endif
