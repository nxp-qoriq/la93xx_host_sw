#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

print_usage()
{
echo "usage: ./iq-replay.sh <iq sample file> <size num 4KB> [half duplex]"
echo "ex : ./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200"
echo "ex : ./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200 1"
}

# check parameters
if [ $# -lt 2 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

if [ $# -gt 2 ];then
	if [ $3 -eq 1 ];then
       		cmd=0x055a0000
	else 
       		cmd=0x05500000
	fi
else
        cmd=0x05500000
fi

(ls $1 >> /dev/null 2>&1)||echo $1 file not found

# check la9310 shiva driver and retrieve iqsample info i.e. iqflood in scratch buffer (non cacheable)
# [] NXP-LA9310-Driver 0000:01:00.0: RFNM IQFLOOD Buff:0xc0000000[H]-0x96400000[M],size 20971520
ddrh=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 2 -d "-"|cut -f 1 -d "["| head -1`
ddrep=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 1 -d "-"|cut -f 1 -d "["| head -1`
maxsize=`dmesg |grep IQFLOOD |cut -f 2 -d "z"|cut -f 2 -d " "| head -1`
if [[ "$ddrh" -eq "" ]];then
        echo can not retrieve IQFLOOD region, is LA9310 shiva started ?
        exit 1
fi
if [ $2 -gt $[$maxsize/4096] ];then
        echo $2 x4KB too large to fit in IQFLOOD region $maxsize bytes
        exit 1
fi

echo bin2mem -f $1 -a $ddrh
bin2mem -f $1 -a $ddrh
cmd=`printf "0x%X\n" $[$cmd + $2]`
#echo vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox recv 0 0

