#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################

print_usage()
{
  echo "usage ./txrx_en.sh <granitatx/granitatxrx/granita2R/limetx/limerx>"
}

if [ $# -eq 0 ];then
  print_usage
  exit 1
else
  if [ $1 = granitatx ];then
    echo "Setting up FDD sma_b Tx "

    echo on > /sys/kernel/rfnm_primary/tx0/enable
    echo sma_b > /sys/kernel/rfnm_primary/tx0/path
    echo 1850000000 > /sys/kernel/rfnm_primary/tx0/freq
    echo 0 > /sys/kernel/rfnm_primary/tx0/rfic_lpf_bw
    echo 1 > /sys/kernel/rfnm_primary/tx0/power
    echo 1 > /sys/kernel/rfnm_primary/tx0/apply

  elif [ $1 = granitatxrx ];then
    echo "Setting up FDD sma_b Tx sma_a Rx"

    echo on > /sys/kernel/rfnm_primary/tx0/enable
    echo sma_b > /sys/kernel/rfnm_primary/tx0/path
    echo 1850000000 > /sys/kernel/rfnm_primary/tx0/freq
    echo 0 > /sys/kernel/rfnm_primary/tx0/rfic_lpf_bw
    echo 1 > /sys/kernel/rfnm_primary/tx0/power
    echo 1 > /sys/kernel/rfnm_primary/tx0/apply

    echo on > /sys/kernel/rfnm_primary/rx1/enable
    echo sma_a > /sys/kernel/rfnm_primary/rx1/path
    echo 1850000000 > /sys/kernel/rfnm_primary/rx1/freq
    echo 100 > /sys/kernel/rfnm_primary/rx1/rfic_lpf_bw
    echo 20 > /sys/kernel/rfnm_primary/rx1/gain
    echo 1 > /sys/kernel/rfnm_primary/rx1/apply

  elif [ $1 = granita2R ];then
    echo "Setting up FDD 2R sma_a and sma_b Rx"

    echo on > /sys/kernel/rfnm_primary/rx1/enable
    echo sma_a > /sys/kernel/rfnm_primary/rx1/path
    echo 1850000000 > /sys/kernel/rfnm_primary/rx1/freq
    echo 100 > /sys/kernel/rfnm_primary/rx1/rfic_lpf_bw
    echo 20 > /sys/kernel/rfnm_primary/rx1/gain
    echo 1 > /sys/kernel/rfnm_primary/rx1/apply
    echo on > /sys/kernel/rfnm_primary/rx0/enable
    echo sma_b > /sys/kernel/rfnm_primary/rx0/path
    echo 1850000000 > /sys/kernel/rfnm_primary/rx0/freq
    echo 100 > /sys/kernel/rfnm_primary/rx0/rfic_lpf_bw
    echo 20 > /sys/kernel/rfnm_primary/rx0/gain
    echo 1 > /sys/kernel/rfnm_primary/rx0/apply

  elif [ $1 = limetx ];then
    echo "Setting up LIME Tx FDD "

    #tx
    echo on > /sys/kernel/rfnm_primary/tx0/enable
    echo sma_b > /sys/kernel/rfnm_primary/tx0/path
    echo 1850000000 > /sys/kernel/rfnm_primary/tx0/freq
    echo 20 > /sys/kernel/rfnm_primary/tx0/rfic_lpf_bw
    echo 10 > /sys/kernel/rfnm_primary/tx0/power
    echo 1 > /sys/kernel/rfnm_primary/tx0/apply

  elif [ $1 = limerx ];then
    echo "Setting up LIME Rx FDD "

    #rx
    echo on > /sys/kernel/rfnm_primary/rx0/enable
    echo sma_a > /sys/kernel/rfnm_primary/rx0/path
    echo 1850000000 > /sys/kernel/rfnm_primary/rx0/freq
    echo 100 > /sys/kernel/rfnm_primary/rx0/rfic_lpf_bw
    echo 20 > /sys/kernel/rfnm_primary/rx0/gain
    echo 1 > /sys/kernel/rfnm_primary/rx0/apply

  else
    print_usage
  fi
fi
