/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2024 NXP
 */
#define IQ_STREAMER_VERSION "b7596b4a65f362755b108de90b47fc5a"
#define  v_DDR_rd_base_address (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002cb8)
#define  p_DDR_rd_base_address (uint32_t)(0x1F000000 + 0x400000 + 0x00002cb8)
#define  s_DDR_rd_base_address (uint32_t)(0x00000004)
#define  v_DDR_rd_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000032ac)
#define  p_DDR_rd_size (uint32_t)(0x1F000000 + 0x400000 + 0x000032ac)
#define  s_DDR_rd_size (uint32_t)(0x00000004)
#define  v_TX_total_produced_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003298)
#define  p_TX_total_produced_size (uint32_t)(0x1F000000 + 0x400000 + 0x00003298)
#define  s_TX_total_produced_size (uint32_t)(0x00000004)
#define  v_TX_total_consumed_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x0000329c)
#define  p_TX_total_consumed_size (uint32_t)(0x1F000000 + 0x400000 + 0x0000329c)
#define  s_TX_total_consumed_size (uint32_t)(0x00000004)
#define  v_output_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00000c00)
#define  p_output_buffer (uint32_t)(0x1F000000 + 0x400000 + 0x00000c00)
#define  s_output_buffer (uint32_t)(0x00002000)
#define  v_TX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003294)
#define  p_TX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x00003294)
#define  s_TX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_rd_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x0000328c)
#define  p_DDR_rd_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x0000328c)
#define  s_DDR_rd_load_start_bit_update (uint32_t)(0x00000004)
#define  v_DDR_rd_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003288)
#define  p_DDR_rd_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x00003288)
#define  s_DDR_rd_start_bit_update (uint32_t)(0x00000004)
#define  v_DDR_wr_base_address (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002ca0)
#define  p_DDR_wr_base_address (uint32_t)(0x1F000000 + 0x400000 + 0x00002ca0)
#define  s_DDR_wr_base_address (uint32_t)(0x00000004)
#define  v_DDR_wr_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002f34)
#define  p_DDR_wr_size (uint32_t)(0x1F000000 + 0x400000 + 0x00002f34)
#define  s_DDR_wr_size (uint32_t)(0x00000004)
#define  v_rx_ch_context (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002f38)
#define  p_rx_ch_context (uint32_t)(0x1F000000 + 0x400000 + 0x00002f38)
#define  s_rx_ch_context (uint32_t)(0x00000110)
#define  v_input_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00004000)
#define  p_input_buffer (uint32_t)(0x1F000000 + 0x500000 + 0x00004000)
#define  s_input_buffer (uint32_t)(0x00003000)
#define  v_RX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002f28)
#define  p_RX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x00002f28)
#define  s_RX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_wr_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002f20)
#define  p_DDR_wr_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x00002f20)
#define  s_DDR_wr_load_start_bit_update (uint32_t)(0x00000004)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003380)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003380)
#define  s_l1_trace_data (uint32_t)(0x00000080)
#define  v_l1_trace_index (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003318)
#define  p_l1_trace_index (uint32_t)(0x1F000000 + 0x400000 + 0x00003318)
#define  s_l1_trace_index (uint32_t)(0x00000004)
#define  v_l1_trace_disable (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003314)
#define  p_l1_trace_disable (uint32_t)(0x1F000000 + 0x400000 + 0x00003314)
#define  s_l1_trace_disable (uint32_t)(0x00000004)
#define  v_g_stats (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002d74)
#define  p_g_stats (uint32_t)(0x1F000000 + 0x400000 + 0x00002d74)
#define  s_g_stats (uint32_t)(0x000000a8)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003380)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003380)
#define  s_l1_trace_data (uint32_t)(0x00000080)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003380)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003380)
#define  s_l1_trace_data (uint32_t)(0x00000080)
#define  v_tx_busy_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000032a0)
#define  p_tx_busy_size (uint32_t)(0x1F000000 + 0x400000 + 0x000032a0)
#define  s_tx_busy_size (uint32_t)(0x00000004)
#define  v_rx_busy_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00002f30)
#define  p_rx_busy_size (uint32_t)(0x1F000000 + 0x400000 + 0x00002f30)
#define  s_rx_busy_size (uint32_t)(0x00000004)
#define TX_NUM_BUF 4
#define TX_DMA_TXR_size	(512)