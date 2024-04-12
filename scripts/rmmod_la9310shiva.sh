#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP
rmmod rfnm_tti
rmmod rfnm_lime
rmmod rfnm_usb
rmmod rfnm_usb_function
rmmod rfnm_daughterboard
rmmod rfnm_gpio
rmmod la9310rfnm
echo 1 > /sys/bus/pci/devices/0000\:00\:00.0/remove
rmmod la9310shiva
echo 1 > /sys/devices/platform/soc\@0/33800000.pcie/pcie_dis

./reset_la9310.sh
sleep 1

echo 0 > /sys/devices/platform/soc\@0/33800000.pcie/pcie_dis
echo 1 > /sys/bus/pci/rescan
