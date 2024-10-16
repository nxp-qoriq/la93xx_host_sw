#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

cmd=`printf "0x%X\n" $[0x06000000]`
#echo vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox send 0 0 $cmd 0
vspa_mbox recv 0 0

cmd=`printf "0x%X\n" $[0x05000000]`
#echo vspa_mbox send 0 0 $cmd $ddrep
vspa_mbox send 0 0 $cmd 0
vspa_mbox recv 0 0
