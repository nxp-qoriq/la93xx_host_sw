#!/bin/bash
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

scratch_addr=0x`ls /sys/firmware/devicetree/base/reserved-memory/ |grep la93|cut -f 2 -d "@"`
[ -f /lib/firmware/la9310_dfe.bin ] || { echo "***ERROR: missing /lib/firmware/la9310_dfe.bin. You are using old BSP, upgrade to BSP 0.4 or above, or rename la9310.bin to la9310_dfe.bin. Boot failed"; exit 1; }
[ -f /lib/firmware/apm-nlm.eld ] || { echo "***ERROR: missing /lib/firmware/apm-nlm.eld. You are using old BSP, upgrade to BSP 0.4 or above, or rename la9310.bin to la9310_dfe.bin. Boot failed"; exit 1; }

# load la9310
echo 7 > /proc/sys/kernel/printk
echo insmod /lib/modules/$(uname -r)/extra/la9310shiva.ko scratch_buf_size=0x4000000 scratch_buf_phys_addr=$scratch_addr adc_mask=0xf adc_rate_mask=0xf dac_mask=0x1 dac_rate_mask=0x1 alt_firmware_name=la9310_dfe.bin alt_vspa_fw_name=apm-nlm.eld
insmod /lib/modules/$(uname -r)/extra/la9310shiva.ko scratch_buf_size=0x4000000 scratch_buf_phys_addr=$scratch_addr adc_mask=0xf adc_rate_mask=0xf dac_mask=0x1 dac_rate_mask=0x1 alt_firmware_name=la9310_dfe.bin alt_vspa_fw_name=apm-nlm.eld

# insert rf modules kernel modules
insmod /lib/modules/$(uname -r)/extra/rfnm_gpio.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_lalib.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_daughterboard.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_lime.ko
insmod /lib/modules/$(uname -r)/extra/rfnm_granita.ko

# enable ADC/DAC always on

#echo dpdk-dfe_app -c "fdd start"
#mount -t hugetlbfs none /dev/hugepages
#echo 24 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
#dpdk-dfe_app -c "fdd start"
# workaround.. until dpdk-dfe_app enables all channels 

la9310_ccsr_base=0x`dmesg | grep BAR:0|cut -f 2 -d "x"|cut -f 1 -d " "`
phytimer_base=$[$la9310_ccsr_base + 0x1020000]
devmem $[$phytimer_base + 0x0c] w 0x0000000a 
devmem $[$phytimer_base + 0x14] w 0x0000000a 
devmem $[$phytimer_base + 0x1c] w 0x0000000a 
devmem $[$phytimer_base + 0x24] w 0x0000000a 
devmem $[$phytimer_base + 0x5c] w 0x0000000a 

# check ADC/DAC clock
#  memtool -32 0x19040300 1
#  00010303 -> 61.44Mhz
#  00000000 -> 122.88Mhz
 

