#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP

# hreset
echo 78 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio78/direction
echo 0 > /sys/class/gpio/gpio78/value

#bootstrap_en
echo 8 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio8/direction
echo 0 > /sys/class/gpio/gpio8/value
 
echo 1 > /sys/class/gpio/gpio78/value
echo 1 > /sys/class/gpio/gpio8/value
sleep 1

echo 78 > /sys/class/gpio/unexport
echo 8 > /sys/class/gpio/unexport

