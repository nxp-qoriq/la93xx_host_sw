#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2023-2024 NXP

# this script takes last System RAM entry in /proc/iomem, adds 1 byte and
# considers that the start of the scratch buffer shared with the LA93xx
# one can run it without parameters to get the base address or with a parameter
# to compute a certain's buffer address:
#
# ./scratch_buff_base.sh
# 0x2360000000
# ./scratch_buff_base.sh 0xa000
# 0x236000A000
# ./scratch_buff_base.sh 1a0000
# 0x23601A0000
# ./scratch_buff_base.sh 1a0000000
# 0x2500000000

ddr_end=`cat /proc/iomem  | grep "System RAM" | tail -n 1 | cut -f 2 -d - | cut -f 1 -d " "`
buff_offset=${1:-0}
if [[ ${buff_offset::2} == "0x" ]]; then
        buff_offset=${buff_offset:2:99}
fi
scratch_start=`echo "obase=16;ibase=16;1+${ddr_end^^}+${buff_offset^^}"|bc`

echo 0x$scratch_start
