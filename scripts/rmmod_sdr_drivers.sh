#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024-2025 NXP

m7_state=$(cat /sys/class/remoteproc/remoteproc0/state)
if [[ "$m7_state" == "running" ]]; then
	memtool -32 0x302F0000=0x000000EA #Stop GPT3 timer
	memtool -32 0x3039000C=000000AA   #Reset M7 core
fi

lsmod | grep sdr_tti
if [[ $? -eq 0 ]]; then
	rmmod sdr_tti
fi

lsmod | grep sdr_lime
if [[ $? -eq 0 ]]; then
	rmmod sdr_lime
fi

lsmod | grep sdr_granita
if [[ $? -eq 0 ]]; then
	rmmod sdr_granita
fi

lsmod | grep sdr_usb
if [[ $? -eq 0 ]]; then
	rmmod sdr_usb
fi

lsmod | grep sdr_usb
if [[ $? -eq 0 ]]; then
	rmmod sdr_usb
fi
lsmod | grep sdr_usb_function
if [[ $? -eq 0 ]]; then
	rmmod sdr_usb_function
fi

lsmod | grep sdr_daughterboard
if [[ $? -eq 0 ]]; then
	rmmod sdr_daughterboard
fi

lsmod | grep sdr_lalib
if [[ $? -eq 0 ]]; then
	rmmod sdr_lalib
fi

lsmod | grep sdr_gpio
if [[ $? -eq 0 ]]; then
	rmmod sdr_gpio
fi

lsmod | grep la9310sdr
if [[ $? -eq 0 ]]; then
	rmmod la9310sdr
fi
