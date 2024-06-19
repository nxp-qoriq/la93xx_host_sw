#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP
memtool -32 0x302F0000=0x000000EA
memtool -32 0x3039000C=000000AA
lsmod | grep rfnm_tti
if [[ $? -eq 0 ]]; then
	rmmod rfnm_tti
fi

lsmod | grep rfnm_lime
if [[ $? -eq 0 ]]; then
	rmmod rfnm_lime
fi

lsmod | grep rfnm_granita
if [[ $? -eq 0 ]]; then
	rmmod rfnm_granita
fi

lsmod | grep rfnm_usb
if [[ $? -eq 0 ]]; then
	rmmod rfnm_usb
fi

lsmod | grep rfnm_usb
if [[ $? -eq 0 ]]; then
	rmmod rfnm_usb
fi
lsmod | grep rfnm_usb_function
if [[ $? -eq 0 ]]; then
	rmmod rfnm_usb_function
fi

lsmod | grep rfnm_daughterboard
if [[ $? -eq 0 ]]; then
	rmmod rfnm_daughterboard
fi

lsmod | grep rfnm_lalib
if [[ $? -eq 0 ]]; then
	rmmod rfnm_lalib
fi

lsmod | grep rfnm_gpio
if [[ $? -eq 0 ]]; then
	rmmod rfnm_gpio
fi

lsmod | grep la9310rfnm
if [[ $? -eq 0 ]]; then
	rmmod la9310rfnm
fi
