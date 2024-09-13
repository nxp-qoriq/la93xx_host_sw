#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

print_usage()
{
echo "usage: ./test-ddr-dma-read <on/off> [half duplex]"
echo "ex : ./test-ddr-dma-read 1"
echo "ex : ./test-ddr-dma-read 0"
echo "ex : ./test-ddr-dma-read 1 1"
}

# check parameters
if [ $# -lt 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# check la9310 shiva driver and retrieve iqsample info i.e. iqflood in scratch buffer (non cacheable)
# [] NXP-LA9310-Driver 0000:01:00.0: RFNM IQFLOOD Buff:0xc0000000[H]-0x96400000[M],size 20971520
ddrh=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 2 -d "-"|cut -f 1 -d "["`
ddrep=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 1 -d "-"|cut -f 1 -d "["`
maxsize=`dmesg |grep IQFLOOD |cut -f 2 -d "z"|cut -f 2 -d " "`
if [[ "$ddrh" -eq "" ]];then
        echo can not retrieve IQFLOOD region, is LA9310 shiva started ?
        exit 1
fi

# build command
if [ $1 -eq 0 ];then
       cmd=0x05000000
else
	if [ $2 -eq 1 ];then
		cmd=`printf "0x%X\n" $[0x05700300]`
	else
		cmd=`printf "0x%X\n" $[0x05300300]`
	fi
fi

vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox recv 0 0
echo running until ./iq-stop.sh and
