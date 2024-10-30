#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#set -x

scratch_addr=0x`hexdump -C /sys/firmware/devicetree/base/reserved-memory/la93@*/reg |cut -f 7-10 -d " " |sed 's/ //g'|cut -f 1 -d " " |head -1`
scratch_size=0x`hexdump -C /sys/firmware/devicetree/base/reserved-memory/la93@*/reg |cut -f 16-19 -d " " |sed 's/ //g'|head -1`

[ -f /lib/firmware/la9310_dfe.bin ] || { echo "***ERROR: missing /lib/firmware/la9310_dfe.bin. You are using old BSP, upgrade to BSP 0.4 or above, or rename la9310.bin to la9310_dfe.bin. Boot failed"; exit 1; }
[ -f /lib/firmware/apm-iqplayer.eld ] || { echo "***ERROR: missing /lib/firmware/apm-iqplayer.eld. You are using old BSP, upgrade to BSP 0.4.3 or above, or rename la9310.bin to la9310_dfe.bin. Boot failed"; exit 1; }

eld_md5sum="$(md5sum /lib/firmware/apm-iqplayer.eld | cut -f 1 -d " ")"
iq_streamer_version="$(iq_streamer -v|cut -f 1 -d " ")"
if [[ "$eld_md5sum" != "$iq_streamer_version" ]]; then
        echo "***ERROR: mismatch between apm-iqplayer.eld md5sum and iq_streamer :" $eld_md5sum "!=" $iq_streamer_version
        exit 1;
fi
adcm=0xf
dacm=0x1
if [ $# -eq 1 ];then
        if [ $1 -eq 1 ];then
                adcm=0
                dacm=0
        fi
        if [ $1 -eq 2 ];then
                adcm=0
                dacm=1
        fi
fi

# load la9310
echo 7 > /proc/sys/kernel/printk
echo insmod /lib/modules/$(uname -r)/extra/la9310shiva.ko scratch_buf_size=$scratch_size scratch_buf_phys_addr=$scratch_addr adc_mask=0xf adc_rate_mask=$adcm dac_mask=0x1 dac_rate_mask=$dacm alt_firmware_name=la9310_dfe.bin alt_vspa_fw_name=apm-iqplayer.eld
insmod /lib/modules/$(uname -r)/extra/la9310shiva.ko scratch_buf_size=$scratch_size scratch_buf_phys_addr=$scratch_addr adc_mask=0xf adc_rate_mask=$adcm dac_mask=0x1 dac_rate_mask=$dacm alt_firmware_name=la9310_dfe.bin alt_vspa_fw_name=apm-iqplayer.eld

# insert rf modules kernel modules
insmod /lib/modules/$(uname -r)/extra/rfnm_gpio.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_lalib.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_daughterboard.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_lime.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_granita.ko
insmod /lib/modules/$(uname -r)/extra/pmu_el0_cycle_counter.ko

# enable ADC/DAC always on

#echo dpdk-dfe_app -c "fdd start"
#mount -t hugetlbfs none /dev/hugepages
#echo 24 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
#dpdk-dfe_app -c "fdd start"
# workaround.. until dpdk-dfe_app enables all channels

la9310_ccsr_base=0x`dmesg | grep BAR:0|cut -f 2 -d "x"|cut -f 1 -d " "| head -1`
phytimer_base=$[$la9310_ccsr_base + 0x1020000]
devmem $[$phytimer_base + 0x0c] w 0x0000000a
devmem $[$phytimer_base + 0x14] w 0x0000000a
devmem $[$phytimer_base + 0x1c] w 0x0000000a
devmem $[$phytimer_base + 0x24] w 0x0000000a
devmem $[$phytimer_base + 0x5c] w 0x0000000a

#
#echo -n "rfnm_m7_v0.elf" > /sys/class/remoteproc/remoteproc0/firmware
#echo stop > /sys/class/remoteproc/remoteproc0/state
#echo start > /sys/class/remoteproc/remoteproc0/state

# check ADC/DAC clock
#  memtool -32 0x19040300 1
#  00010303 -> 61.44Mhz
#  00000000 -> 122.88Mhz


