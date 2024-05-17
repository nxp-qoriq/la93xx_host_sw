#!/bin/sh
#SPDX-License-Identifier: BSD-3-Clause
#Copyright 2024 NXP

gpioget 2 9
gpioget 2 14

./rfnm_enable_la9310_uart_ext_port

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
insmod /lib/modules/$(uname -r)/extra/rfnm_gpio.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_lalib.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_daughterboard.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_usb_function.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_usb.ko file=/home/root/backing_storage

sleep 1

insmod /lib/modules/$(uname -r)/extra/rfnm_lime.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_granita.ko
echo -n "rfnm_m7_v0.elf" > /sys/class/remoteproc/remoteproc0/firmware
echo stop > /sys/class/remoteproc/remoteproc0/state
echo start > /sys/class/remoteproc/remoteproc0/state

memtool 0x303D0968=0xff

sleep 1

cd fr1_fr2_test_tool

