#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024-2025 NXP

insmod /lib/modules/$(uname -r)/extra/sdr_lime.ko
echo -n "sdr_m7_v0.elf" > /sys/class/remoteproc/remoteproc0/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo start > /sys/class/remoteproc/remoteproc0/state

memtool 0x303D0968=0xff
