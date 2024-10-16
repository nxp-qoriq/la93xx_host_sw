#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

print_usage()
{
echo "usage: ./iq-capture.sh <DDR buff size nb 4KB> [half duplex]"
echo "ex : ./iq-capture.sh ./iqdata.bin 300"
echo "ex : ./iq-capture.sh ./iqdata.bin 300 1"
}

# check parameters
if [ $# -lt 2 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# check la9310 shiva driver and retrieve iqsample info i.e. iqflood in scratch buffer (non cacheable)
# [] NXP-LA9310-Driver 0000:01:00.0: RFNM IQFLOOD Buff:0xc0000000[H]-0x96400000[M],size 20971520
ddrh=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 2 -d "-"|cut -f 1 -d "["| head -1`
ddrep=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 1 -d "-"|cut -f 1 -d "["| head -1`
maxsize=`dmesg |grep IQFLOOD |cut -f 2 -d ","|cut -f 2 -d " "| head -1`
buff=`printf "0x%X\n" $[$maxsize/2 + $ddrh]`
buffep=`printf "0x%X\n" $[$maxsize/2 + $ddrep]`
if [[ "$ddrh" -eq "" ]];then
        echo can not retrieve IQFLOOD region, is LA9310 shiva started ?
        exit 1
fi
if [ $2 -gt $[$maxsize/2/4096] ];then
        echo $2 x4KB too large to fit in IQFLOOD region $maxsize bytes
        exit 1
fi

if [ $# -gt 2 ];then
	if [ $3 -eq 1 ];then
		cmd=`printf "0x%X\n" $[0x06520000 + $2]`
	else 
		cmd=`printf "0x%X\n" $[0x06500000 + $2]`
	fi
else
	cmd=`printf "0x%X\n" $[0x06500000 + $2]`
fi

vspa_mbox send 0 0 $cmd $buffep
vspa_mbox recv 0 0
echo bin2mem -f $1 -a $buff -r $[4096 * $2]
bin2mem -f $1 -a $buff -r $[4096 * $2]

