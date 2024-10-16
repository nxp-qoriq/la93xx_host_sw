/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2024 NXP
 */
#define IQ_STREAMER_VERSION "2fbb9d4993a958652c9f6bac3f5d0cae"
#define  v_DDR_rd_base_address (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000038b8)
#define  p_DDR_rd_base_address (uint32_t)(0x1F000000 + 0x400000 + 0x000038b8)
#define  s_DDR_rd_base_address (uint32_t)(0x00000004)
#define  v_DDR_rd_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003cac)
#define  p_DDR_rd_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003cac)
#define  s_DDR_rd_size (uint32_t)(0x00000004)
#define  v_TX_total_produced_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003c98)
#define  p_TX_total_produced_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003c98)
#define  s_TX_total_produced_size (uint32_t)(0x00000004)
#define  v_TX_total_consumed_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003c9c)
#define  p_TX_total_consumed_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003c9c)
#define  s_TX_total_consumed_size (uint32_t)(0x00000004)
#define  v_output_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00001800)
#define  p_output_buffer (uint32_t)(0x1F000000 + 0x400000 + 0x00001800)
#define  s_output_buffer (uint32_t)(0x00002000)
#define  v_TX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003c94)
#define  p_TX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x00003c94)
#define  s_TX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_rd_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003c8c)
#define  p_DDR_rd_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x00003c8c)
#define  s_DDR_rd_load_start_bit_update (uint32_t)(0x00000004)
#define  v_DDR_rd_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003c88)
#define  p_DDR_rd_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x00003c88)
#define  s_DDR_rd_start_bit_update (uint32_t)(0x00000004)
#define  v_DDR_wr_base_address (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000038a0)
#define  p_DDR_wr_base_address (uint32_t)(0x1F000000 + 0x400000 + 0x000038a0)
#define  s_DDR_wr_base_address (uint32_t)(0x00000004)
#define  v_DDR_wr_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003ab4)
#define  p_DDR_wr_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003ab4)
#define  s_DDR_wr_size (uint32_t)(0x00000004)
#define  v_rx_ch_context (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003ab8)
#define  p_rx_ch_context (uint32_t)(0x1F000000 + 0x400000 + 0x00003ab8)
#define  s_rx_ch_context (uint32_t)(0x00000088)
#define  v_input_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00004000)
#define  p_input_buffer (uint32_t)(0x1F000000 + 0x500000 + 0x00004000)
#define  s_input_buffer (uint32_t)(0x00003000)
#define  v_RX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003aa8)
#define  p_RX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x00003aa8)
#define  s_RX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_wr_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003aa0)
#define  p_DDR_wr_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x00003aa0)
#define  s_DDR_wr_load_start_bit_update (uint32_t)(0x00000004)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003d80)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003d80)
#define  s_l1_trace_data (uint32_t)(0x00000070)
#define  v_l1_trace_index (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003d18)
#define  p_l1_trace_index (uint32_t)(0x1F000000 + 0x400000 + 0x00003d18)
#define  s_l1_trace_index (uint32_t)(0x00000004)
#define  v_l1_trace_disable (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003d14)
#define  p_l1_trace_disable (uint32_t)(0x1F000000 + 0x400000 + 0x00003d14)
#define  s_l1_trace_disable (uint32_t)(0x00000004)
#define  v_g_stats (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003970)
#define  p_g_stats (uint32_t)(0x1F000000 + 0x400000 + 0x00003970)
#define  s_g_stats (uint32_t)(0x00000068)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003d80)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003d80)
#define  s_l1_trace_data (uint32_t)(0x00000070)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003d80)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003d80)
#define  s_l1_trace_data (uint32_t)(0x00000070)
#define  v_tx_busy_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003ca0)
#define  p_tx_busy_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003ca0)
#define  s_tx_busy_size (uint32_t)(0x00000004)
#define  v_rx_busy_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003ab0)
#define  p_rx_busy_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003ab0)
#define  s_rx_busy_size (uint32_t)(0x00000004)
#define TX_NUM_BUF 4
#define TX_DMA_TXR_size	(512)
