#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x
print_usage()
{
echo "usage: ./test-axiq-dma-read.sh <on/off>"
echo "   ex: ./test-axiq-dma-read.sh 1"

}

# check parameters
if [ $# -ne 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

# build command
if [ $1 == 0 ];then
       cmd=0x01000000
else 
       cmd=0x01300000
fi

#echo vspa_mbox send 0 0 $cmd 0x20
vspa_mbox send 0 0 $cmd 0x20
vspa_mbox recv 0 0


