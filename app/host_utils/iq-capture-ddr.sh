#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

print_usage()
{
echo "usage: ./iq-capture-c.sh <size num 4KB>"
echo "ex : ./iq-capture-ddr.sh 300"
}

# check parameters
if [ $# -ne 1 ];then
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
if [ $1 -gt $[$maxsize/4096] ];then
        echo $1 x4KB too large to fit in IQFLOOD region $maxsize bytes
        exit 1
fi

cmd=`printf "0x%X\n" $[0x06900000 + $1]`
#echo vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox recv 0 0
echo after ./iq-stop.sh need to do :
echo bin2mem -f iqdata.bin -a $ddrh -r $[4096 * $1]

