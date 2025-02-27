/* SPDX-License-Identifier: BSD-3-Clause
* Copyright 2024 NXP
 */
#define IQ_STREAMER_VERSION "d9ff315d9c5815477ae71d94609ecc27"
#define  v_ddr_rd_dma_xfr_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00000114)
#define  p_ddr_rd_dma_xfr_size (uint32_t)(0x1F000000 + 0x400000 + 0x00000114)
#define  s_ddr_rd_dma_xfr_size (uint32_t)(0x00000004)
#define  v_ddr_wr_dma_xfr_size (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000000f0)
#define  p_ddr_wr_dma_xfr_size (uint32_t)(0x1F000000 + 0x400000 + 0x000000f0)
#define  s_ddr_wr_dma_xfr_size (uint32_t)(0x00000004)
#define  v_output_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00006000)
#define  p_output_buffer (uint32_t)(0x1F000000 + 0x500000 + 0x00006000)
#define  s_output_buffer (uint32_t)(0x00002000)
#define  v_TX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000033e0)
#define  p_TX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x000033e0)
#define  s_TX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_rd_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000033d8)
#define  p_DDR_rd_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x000033d8)
#define  s_DDR_rd_load_start_bit_update (uint32_t)(0x00000004)
#define  v_input_qec_buffer (volatile uint32_t *)((uint64_t)BAR2_addr + 0x500000 + 0x00004000)
#define  p_input_qec_buffer (uint32_t)(0x1F000000 + 0x500000 + 0x00004000)
#define  s_input_qec_buffer (uint32_t)(0x00002000)
#define  v_RX_ext_dma_enabled (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x000033ac)
#define  p_RX_ext_dma_enabled (uint32_t)(0x1F000000 + 0x400000 + 0x000033ac)
#define  s_RX_ext_dma_enabled (uint32_t)(0x00000004)
#define  v_DDR_wr_load_start_bit_update (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x0000339c)
#define  p_DDR_wr_load_start_bit_update (uint32_t)(0x1F000000 + 0x400000 + 0x0000339c)
#define  s_DDR_wr_load_start_bit_update (uint32_t)(0x00000004)
#define  v_l1_trace_data (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003480)
#define  p_l1_trace_data (uint32_t)(0x1F000000 + 0x400000 + 0x00003480)
#define  s_l1_trace_data (uint32_t)(0x00000640)
#define  v_l1_trace_index (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003408)
#define  p_l1_trace_index (uint32_t)(0x1F000000 + 0x400000 + 0x00003408)
#define  s_l1_trace_index (uint32_t)(0x00000004)
#define  v_l1_trace_disable (volatile uint32_t *)((uint64_t)BAR2_addr + 0x400000 + 0x00003404)
#define  p_l1_trace_disable (uint32_t)(0x1F000000 + 0x400000 + 0x00003404)
#define  s_l1_trace_disable (uint32_t)(0x00000004)
