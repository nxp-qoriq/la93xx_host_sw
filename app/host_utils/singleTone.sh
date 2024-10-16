#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

print_usage()
{
echo "usage: ./singleTone.sh <fft-bin> <on/off>"
echo "   ex: ./singleTone.sh 42 1"
echo "            tone index ((freq (in MHz) * 512) / Sampling rate)"
echo ""
echo "            is 2 byte long, its value should be:"
echo "            for 61.44 msps sampling rate:"
echo "                417 ~ 50 Mhz"
echo "                333 ~ 40 Mhz"
echo "                167 ~ 20 Mhz"
echo "                 83 ~ 10 MHz"
echo "                 42 ~ 5 MHz"
echo "                 20 ~ 2.5 MHz"
echo ""
echo "            for 122.888 msps sampling rate:"
echo "                208 ~ 50 Mhz"
echo "                125 ~ 30 Mhz"
echo "                 42 ~ 10 MHz"
echo "                 21 ~ 5 MHz"
echo "                 10 ~ 2.5 MHz"

}

# check parameters
if [ $# -ne 2 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# build command

if [ $2 == 1 ];then
       cmd=0x01100000
       #cmd=0x01500000 // bank swap
else 
       cmd=0x01000000
fi

val=`printf "0x%X" $1`

#echo vspa_mbox send 0 0 $cmd $val
vspa_mbox send 0 0 $cmd $val
vspa_mbox recv 0 0


