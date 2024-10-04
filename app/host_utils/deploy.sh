#!/bin/sh
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#set -x

print_usage()
{
echo "usage: ./deploy.sh <target ipaddr>"
echo "ex : ./deploy.sh 172.16.0.156"
}

# check parameters
if [ $# -ne 1 ];then
        echo Arguments wrong.
        print_usage
        exit 1
fi

scp -r host_utils root@$1:/home/root
scp host_utils/fw_iqplayer/apm-iqplayer.eld root@$1:/lib/firmware
scp host_utils/iq_streamer/iq_streamer root@$1:/usr/bin

