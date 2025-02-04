/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 *
 * Copyright 2024-2025 NXP
 */

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sdr-shared.h>
#include "sdr_fe_mt3812.h"
#include "rfnm_fe_generic.h"

//TX On function where fem signals are set based on TX path and gain
void mt3812_tx_on(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch, int high_gain)
{
	pr_debug("%s\n", __func__);
	if (high_gain == 1) {//high gain
		if (tx_ch->path == RFNM_PATH_SMA_A)
			rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 1);//Goes through ANT A
		else
			rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 0);//Goes through ANT B
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_A, 1);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_B, 0);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_TX_TLI, 0);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_TX_TLO, 1);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 0);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_PA_EN, 1);
	} else {//low gain
		rfnm_fe_srb(dgb_dt, SDR_MT3812_PA_EN, 0);
		if (tx_ch->path == RFNM_PATH_SMA_A)
			rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 1);//Goes through ANT A
		else
			rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 0);//Goes through ANT B
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_A, 1);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_B, 0);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_TX_TLI, 1);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_TX_TLO, 0);
		rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 0);
	}
}

//TX off function where fem signals are set based on TX path
void mt3812_tx_off(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch)
{
	pr_debug("%s\n", __func__);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_PA_EN, 0);
	if (tx_ch->path == RFNM_PATH_SMA_A)
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 1);//Goes through ANT A
	else
		rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_T, 0);//Goes through ANT B
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_A, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_B, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 1);
}

//RX ANT_B On function where fem signals are set based on gain
void mt3812_rx_ant_b_on(struct rfnm_dgb *dgb_dt, int high_gain)
{
	pr_debug("%s\n", __func__);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_B, 1);
	if (high_gain == 1)
		rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_B_LNA_BYP, 0);
	else
		rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_B_LNA_BYP, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_B_LNA_ENB, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 1);
}

//RX Ant_B Off function where fem signals are set to 0
void mt3812_rx_ant_b_off(struct rfnm_dgb *dgb_dt)
{
	pr_debug("%s\n", __func__);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_B, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_B_LNA_ENB, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_B_LNA_BYP, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 0);
}

//RX Ant_A On  function where fem signals are set based on gain
void mt3812_rx_ant_a_on(struct rfnm_dgb *dgb_dt, int high_gain)
{
	pr_debug("%s\n", __func__);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_A, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_A_LNA_ENB, 0);
	if (high_gain == 1)
		rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_A_LNA_BYP, 0);
	else
		rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_A_LNA_BYP, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 1);
}

//RX ANT_A Off function where fem signals are set
void mt3812_rx_ant_a_off(struct rfnm_dgb *dgb_dt)
{
	pr_debug("%s\n", __func__);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_MT_TRX, 0);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_A_LNA_ENB, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_FF_ANT_A, 1);
	rfnm_fe_srb(dgb_dt, SDR_MT3812_ANT_A_LNA_BYP, 0);
}
