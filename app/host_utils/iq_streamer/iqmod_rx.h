// =============================================================================
//! @file       iqmod_rx.h
//! @brief      Receive IQ samples
//! @author     NXP Semiconductors.
//! @copyright  Copyright 2023-2024 NXP
// =============================================================================
/*
*  NXP Confidential. This software is owned or controlled by NXP and may only be used strictly
*  in accordance with the applicable license terms. By expressly accepting
*  such terms or by downloading, installing, activating and/or otherwise using
*  the software, you are agreeing that you have read, and that you agree to
*  comply with and are bound by, such license terms. If you do not agree to
*  be bound by the applicable license terms, then you may not retain,
*  install, activate or otherwise use the software.
*/

#ifndef IQMOS_RX_H_
#define IQMOS_RX_H_

#ifdef IQMOD_RX_1R
#define RX_NUM_CHAN 1
#define RX_NUM_BUF 5
#define RX_DMA_TXR_size	(512)
#define RX_DECIM 1
#define RX_DMA_TXR_STEP	(4*RX_DMA_TXR_size)
#define RX_DDR_STEP		(RX_DMA_TXR_STEP/RX_DECIM)  
#endif

#ifdef IQMOD_RX_2R
#define RX_NUM_CHAN 2
#define RX_NUM_BUF 3
#define RX_NUM_DEC_BUF 3
#define RX_DMA_TXR_size	(512)
#define RX_DECIM 2
#define RX_DMA_TXR_STEP	(4*RX_DMA_TXR_size)
#define RX_DDR_STEP		(RX_DMA_TXR_STEP/RX_DECIM)  
#endif

#ifdef IQMOD_RX_4R
#define RX_NUM_CHAN 4
#define RX_NUM_BUF 3
#define RX_NUM_DEC_BUF 3
#define RX_DMA_TXR_size	(256)
#define RX_DECIM 4
#define RX_DMA_TXR_STEP	(4*RX_DMA_TXR_size)
#define RX_DDR_STEP		(RX_DMA_TXR_STEP/RX_DECIM)  
#endif


#ifdef __VSPA__

#include "txiqcomp.h"


#ifdef IQMOD_RX_1R

extern vspa_complex_fixed16 input_buffer[RX_NUM_BUF*RX_DMA_TXR_size]	__attribute__(( section(".ippu_dmem") )) __attribute__ ((aligned(64)));
extern vspa_complex_fixed16 *input_buffer_0;
extern vspa_complex_fixed16 *input_buffer_1;

#define INCR_RX_BUFF(rxbuff_ptr) {\
		/*rxbuff_ptr##_prev=rxbuff_ptr;*/\
		rxbuff_ptr += RX_DMA_TXR_size;\
		if (rxbuff_ptr >=  &input_buffer[RX_NUM_BUF*RX_DMA_TXR_size]) \
		{rxbuff_ptr =  &input_buffer[0]; } \
}


#else /*#ifdef IQMOD_RX_1R*/

extern vspa_complex_fixed16 input_buffer[RX_NUM_CHAN][RX_NUM_BUF*RX_DMA_TXR_size]	__attribute__(( section(".ippu_dmem") )) __attribute__ ((aligned(64)));

#define INCR_RX_BUFF(rxbuff_ptr,chan) {\
		/*rxbuff_ptr##_prev=rxbuff_ptr;*/\
		rxbuff_ptr += RX_DMA_TXR_size;\
		if (rxbuff_ptr >=  &input_buffer[chan][RX_NUM_BUF*RX_DMA_TXR_size]) \
		{rxbuff_ptr =  &input_buffer[chan][0]; } \
}
#define INCR_RX_DDR_BUFF(rxbuff_ptr,chan) {\
		/*rxbuff_ptr##_prev=rxbuff_ptr;*/\
		rxbuff_ptr += RX_DMA_TXR_size/RX_DECIM;\
		if (rxbuff_ptr >=  &input_dec_buffer[chan][RX_NUM_BUF*RX_DMA_TXR_size/RX_DECIM]) \
		{rxbuff_ptr =  &input_dec_buffer[chan][0]; } \
}

#endif

typedef struct rx_ch_context_s {
	uint32_t RX_index;
	uint32_t RX_total_axiq_enqueued_size;  
	uint32_t RX_total_axiq_received_size;
	uint32_t RX_total_dmem_QECed_size;     /* RX_total_produced_size; */
	uint32_t RX_total_dmem_input_Decimated_size; /* RX_total_produced_size; */
	uint32_t RX_total_dmem_output_Decimated_size; /* RX_total_produced_size; */
	uint32_t RX_total_ddr_enqueued_size;
	uint32_t RX_total_ddr_consumed_size;   /* RX_total_consumed_size */
	vspa_complex_fixed16 * p_rx_axiq_enqueued;
	vspa_complex_fixed16 * p_rx_axiq_received;
	vspa_complex_fixed16 * p_rx_dmem_QECed;
	vspa_complex_fixed16 * p_rx_dmem_input_decimated;
	vspa_complex_fixed16 * p_rx_dmem_output_decimated;
	vspa_complex_fixed16 * p_rx_ddr_enqueued;
	vspa_complex_fixed16 * p_rx_consumed;
	uint32_t DDR_wr_offset;
	uint32_t DDR_wr_base_address;

} rx_ch_context_t;

extern structTXIQCompParams rxiqcompcfg_struct _VSPA_VECTOR_ALIGN;

void DDR_write(uint32_t DDR_wr_dma_channel, uint32_t DDR_address, uint32_t vsp_address);
void rx_qec_correction(vspa_complex_fixed16 *data);
void RX_IQ_DATA_TO_DDR(void);
void PUSH_RX_DATA(void);

extern uint32_t DDR_wr_QEC_enable;
extern uint32_t DDR_wr_start_bit_update, DDR_wr_load_start_bit_update, DDR_wr_continuous;


#define DDR_WR_DMA_CHANNEL_1  0xc
#define DDR_WR_DMA_CHANNEL_2  0xd
#define DDR_WR_DMA_CHANNEL_3  0xe
#define DDR_WR_DMA_CHANNEL_4  0xf

extern uint32_t DDR_wr_base_address;
extern uint32_t DDR_wr_dma_channel_underrun;

#endif /* #ifdef __VSPA__ */

#endif /* IQMOS_RX_H_ */
