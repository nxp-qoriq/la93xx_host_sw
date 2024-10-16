#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

print_usage()
{
echo "usage: ./test-ddr-dma-write.sh <on/off> [dma config]"
echo "ex : ./test-ddr-dma-write.sh 1"
echo "ex : ./test-ddr-dma-write.sh 0"
echo "ex : dma config : 1-2 nb "
echo "ex : ./test-ddr-dma-read 1 2"
}

# check parameters
if [ $# -lt 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# check la9310 shiva driver and retrieve iqsample info i.e. iqflood in scratch buffer (non cacheable)
# [] NXP-LA9310-Driver 0000:01:00.0: RFNM IQFLOOD Buff:0xc0000000[H]-0x96400000[M],size 20971520
ddrh=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 2 -d "-"|cut -f 1 -d "["| head -1`
ddrep=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 1 -d "-"|cut -f 1 -d "["| head -1`
maxsize=`dmesg |grep IQFLOOD |cut -f 2 -d ","|cut -f 2 -d " "| head -1`
if [[ "$ddrh" -eq "" ]];then
        echo can not retrieve IQFLOOD region, is LA9310 shiva started ?
        exit 1
fi

# build command
if [ $1 == 0 ];then
	cmd=0x06000000
else 
	cmd=`printf "0x%X\n" $[0x06300300]`
	if [ $# -eq 2 ];then
		cmd=`printf "0x%X\n" $[0x06300300 + $2*0x10000]`
	fi
fi


vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox recv 0 0
echo running until ./iq-stop.sh and 

