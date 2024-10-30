#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

echo -n "iqstreamer_m7_v0.elf" > /sys/class/remoteproc/remoteproc0/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo start > /sys/class/remoteproc/remoteproc0/state
#m7_state=$(cat /sys/class/remoteproc/remoteproc0/state)
#if [[ "$m7_state" == "running" ]]; then
#        memtool -32 0x302F0000=0x000000EA #Stop GPT3 timer
#        memtool -32 0x3039000C=000000AA   #Reset M7 core
#fi


