#!/bin/bash
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

print_usage()
{
echo "usage: ./iq-replay.sh <capture file> <size num 4KB>"
echo "ex : ./iq-capture.sh ./iqdata.bin 300"
}

# check vspa suitable firmware
#diff=`diff /lib/firmware/apm.eld /lib/firmware/apm-nlm.eld`
#if [ "$diff" != ""  ];then
#        echo "need to deploy nlm firmware i.e. [cp /lib/firmware/apm-nlm.eld /lib/firmware/apm.eld]"
#        exit 1
#fi

# check parameters
if [ $# -ne 2 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# check la9310 shiva driver and retrieve iqsample info i.e. iqflood in scratch buffer (non cacheable)
# [] NXP-LA9310-Driver 0000:01:00.0: RFNM IQFLOOD Buff:0xc0000000[H]-0x96400000[M],size 20971520
ddrh=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 2 -d "-"|cut -f 1 -d "["`
ddrep=`dmesg |grep IQFLOOD|cut -f 5 -d ":"|cut -f 1 -d "-"|cut -f 1 -d "["`
maxsize=`dmesg |grep IQFLOOD |cut -f 2 -d ","|cut -f 2 -d " "`
if [[ "$ddrh" -eq "" ]];then
        echo can not retrieve IQFLOOD region, is LA9310 shiva started ?
        exit 1
fi
if [ $2 -gt $[$maxsize/4096] ];then
        echo $2 x4KB too large to fit in IQFLOOD region $maxsize bytes
        exit 1
fi

cmd=`printf "0x%X\n" $[0x06100000 + $2]`
#echo vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox recv 0 0
echo bin2mem -f $1 -a $ddrh -r $[4096 * $2]
bin2mem -f $1 -a $ddrh -r $[4096 * $2]

