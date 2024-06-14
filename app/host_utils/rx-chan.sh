#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

print_usage()
{
echo "usage: ./rx-chan.sh <chanID> "
echo " primary slot   : sma_a (3) sma_b (1)"
echo " secondary slot : sma_a (2) sma_b (0)"
echo " sma_a is at the edge of primary slot "
}

# check parameters
if [ $# -ne 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

#                                                            >default<
# chanID                  1           2           3           4
# VSPA chanID             0           1           2           3
# axi_ADC_FIFI_addr[4] = {0x44003000, 0x44004000, 0x44001000, 0x44002000};
# LA9310 chan             RX0         RX1         RO0         RO1
# RFNM slot_chan          RBB_RX2     RBA_RX2     RBB_RX1     RBA_RX1
# GRANITA                 sma_a       sma_a       sma_b       sma_b
# LIME                                    		  sma_b	      sma_b	

cmd=`printf "0x%X\n" $[0x0d000000]`
#echo vspa_mbox send 0 0 $cmd $1
vspa_mbox send 0 0 $cmd $1
vspa_mbox recv 0 0

