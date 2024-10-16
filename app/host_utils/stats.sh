#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

print_usage()
{
echo "usage: ./stats.sh counterID"
echo "	STAT_DMA_AXIQ_WRITE (x2KB)     // 0x0"
echo "	STAT_DMA_AXIQ_READ  (x2KB)     // 0x1"
echo "	STAT_DMA_DDR_RD     (x2KB)     // 0x2"
echo "	STAT_DMA_DDR_WR     (x2KB)     // 0x3"
echo "	STAT_EXT_DMA_DDR_RD (x2KB)     // 0x4"
echo "	STAT_EXT_DMA_DDR_WR (x2KB)     // 0x5"
echo "	ERROR_DMA_DDR_RD_UNDERRUN      // 0x6"
echo "	ERROR_DMA_DDR_WR_UNDERRUN      // 0x7"
echo "	ERROR_AXIQ_FIFO_TX_UNDERRUN    // 0x8"
echo "	ERROR_AXIQ_FIFO_RX_OVERRUN     // 0x9"
echo "	ERROR_AXIQ_DMA_TX_CMD_UNDERRUN // 0xA"
echo "	ERROR_AXIQ_DMA_RX_CMD_UNDERRUN // 0xB"
echo "	ERROR_DMA_CONFIG_ERROR         // 0xC"
echo "	ERROR_DMA_XFER_ERROR           // 0xD"
}

# check parameters
if [ $# -ne 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# build command
if [ $1 == -1 ];then
       cmd=0x0f100000
else 
       cmd=`printf "0x%X\n" $[0x0f000000+$1]`
fi

#echo vspa_mbox send 0 0 $cmd 0 
vspa_mbox send 0 0 $cmd 0
vspa_mbox recv 0 0

