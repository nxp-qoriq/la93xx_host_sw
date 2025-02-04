#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024-2025 NXP

gpioget 2 9
gpioget 2 14

./sdr_enable_la9310_uart_ext_port

#lspci -vv | grep Addr
#echo 1 > /sys/bus/pci/devices/0000\:01\:00.0/remove
#sleep 1
#lspci
#echo "1" > /sys/bus/pci/rescan

lspci -vv | grep Addr

echo 7 > /proc/sys/kernel/printk
unlink /home/root/backing_storage
dd if=/dev/zero of=/home/root/backing_storage bs=1M count=64
cd fr1_fr2_test_tool
echo $1
./boot_vspa.sh rc30
cd ..
insmod /lib/modules/$(uname -r)/extra/sdr_gpio.ko
insmod /lib/modules/$(uname -r)/extra/sdr_lalib.ko
insmod /lib/modules/$(uname -r)/extra/sdr_daughterboard.ko
insmod /lib/modules/$(uname -r)/extra/sdr_usb_function.ko
insmod /lib/modules/$(uname -r)/extra/sdr_usb.ko file=/home/root/backing_storage

sleep 1

insmod /lib/modules/$(uname -r)/extra/sdr_lime.ko
echo -n "sdr_m7_v0.elf" > /sys/class/remoteproc/remoteproc0/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo start > /sys/class/remoteproc/remoteproc0/state

memtool 0x303D0968=0xff

sleep 1

cd fr1_fr2_test_tool

