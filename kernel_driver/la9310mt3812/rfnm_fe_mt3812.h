/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 *
 * Copyright 2024 NXP
 *
 */
#ifndef RFNM_MT3812_H_
#define RFNM_MT3812_H_
#include <linux/rfnm-shared.h>

//The following TX function declarations are for "mt3812 TX on" containing high gain and
//low gain, "mt3812 TX off" where fem signals values are set based on execution sequence
//The following RX function declarations are for "mt3812 RX on" for ant_A and ant_B,
//"mt3812 RX off" for ant_A and ant_B where fem signals values are set based on execution sequence
void mt3812_tx_on(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch, int gain);
void mt3812_tx_off(struct rfnm_dgb *dgb_dt, struct rfnm_api_tx_ch *tx_ch);
void mt3812_rx_ant_b_on(struct rfnm_dgb *dgb_dt, int gain);
void mt3812_rx_ant_b_off(struct rfnm_dgb *dgb_dt);
void mt3812_rx_ant_a_on(struct rfnm_dgb *dgb_dt, int gain);
void mt3812_rx_ant_a_off(struct rfnm_dgb *dgb_dt);
int rfnm_reset(struct rfnm_dgb *dgb_dt);

typedef struct {
	struct fe_s fe_tx_full;
	struct fe_s fe_tx_bypass;
	struct fe_s fe_rx_full;
	struct fe_s fe_rx_bypass;
	int tx_gain_db;
	int rx_gain_db;
} mt3812_priv_t;

#define MT3812_RXGAIN_NO_LNA 33
#define MT3812_TXGAIN_NO_PA 33
#define RFNM_MT3812_NUM_LATCHES_1 1
#define RFNM_MT3812_NUM_LATCHES_2 1
#define RFNM_MT3812_NUM_LATCHES_3 1
#define RFNM_MT3812_NUM_LATCHES_4 0
#define RFNM_MT3812_NUM_LATCHES_5 0
#define RFNM_MT3812_NUM_LATCHES_6 0
#define RFNM_MT3812_NUM_LATCHES ((int[]){0, RFNM_MT3812_NUM_LATCHES_1, RFNM_MT3812_NUM_LATCHES_2, RFNM_MT3812_NUM_LATCHES_3, RFNM_MT3812_NUM_LATCHES_4, RFNM_MT3812_NUM_LATCHES_5, RFNM_MT3812_NUM_LATCHES_6})

#define RFNM_LATCH1 (1 << 28)
#define RFNM_LATCH2 (2 << 28)
#define RFNM_LATCH3 (3 << 28)
#define RFNM_LATCH4 (4 << 28)
#define RFNM_LATCH5 (5 << 28)
#define RFNM_LATCH6 (6 << 28)

#define RFNM_LATCH_SEQ1 (1 << 24)
#define RFNM_LATCH_SEQ2 (2 << 24)
#define RFNM_LATCH_SEQ3 (3 << 24)

#define RFNM_LATCH_Q0 (0 << 16)
#define RFNM_LATCH_Q1 (1 << 16)
#define RFNM_LATCH_Q2 (2 << 16)
#define RFNM_LATCH_Q3 (3 << 16)
#define RFNM_LATCH_Q4 (4 << 16)
#define RFNM_LATCH_Q5 (5 << 16)
#define RFNM_LATCH_Q6 (6 << 16)
#define RFNM_LATCH_Q7 (7 << 16)

//The following headers for FEM signals are defined based on RFNM MT3812 latch number, latch sequence and output
#define RFNM_MT3812_TX_TLO (RFNM_LATCH1 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q5)
#define RFNM_MT3812_PA_EN (RFNM_LATCH1 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q6)
#define RFNM_MT3812_TX_TLI (RFNM_LATCH1 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q7)

#define RFNM_MT3812_ANT_A_LNA_ENB (RFNM_LATCH2 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q1)
#define RFNM_MT3812_ANT_A_LNA_BYP (RFNM_LATCH2 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q3)
#define RFNM_MT3812_ANT_B_LNA_BYP (RFNM_LATCH2 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q5)
#define RFNM_MT3812_ANT_B_LNA_ENB (RFNM_LATCH2 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q7)

#define RFNM_MT3812_MT_TRX (RFNM_LATCH3 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q0)
#define RFNM_MT3812_FF_ANT_B (RFNM_LATCH3 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q1)
#define RFNM_MT3812_FF_ANT_T (RFNM_LATCH3 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q2)
#define RFNM_MT3812_FF_ANT_A (RFNM_LATCH3 | RFNM_LATCH_SEQ1 | RFNM_LATCH_Q5)
#endif
