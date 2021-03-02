#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2017, 2021-2022 NXP

#Remove the card
echo 1 > /sys/bus/pci/devices/0001\:01\:00.0/remove
sleep 1

#cold reset
echo 378 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio378/direction
echo 0 > /sys/class/gpio/gpio378/value
echo 1 > /sys/class/gpio/gpio378/value
sleep 1

#rescan the card
echo 1 > /sys/bus/pci/devices/0001\:00\:00.0/rescan
