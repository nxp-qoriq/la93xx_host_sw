# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################

###################################################
# singleTone play/capture
#
# scripts are compatible with apm-iqplayer.eld
# File size is num of 4KB buffer
###################################################

################################################################################
## Basic enablement : “How to transmit/receive waveform from/to binary files ?”
#  iq_streamer is only used to display statistics, debug and host/vspa traces. 
#  So far VSPA firmware is self-content  and we are using VSPA DMAs only, so no need for imx agent/DMA


# play (repeat) single tone on granita
./load-nlm.sh
./config_run_rf.sh granitatx
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

# capture (one time) iq samples into file
./load-nlm.sh
./config_run_rf.sh granitarx
./iq-capture.sh ./iqdata.bin 300

# capture (repeat) in DDR buffer 
./load-nlm.sh
./config_run_rf.sh granitarx
./iq-capture-ddr.sh 300
./iq-stop.sh
bin2mem -f 300 -a 0x9CC00000 -r 1228800

# play and capture single tone on Granita
./load-nlm.sh
mount -t hugetlbfs none /dev/hugepages
echo 24 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
dpdk-dfe_app -c "axiq_lb enable"
./singleTone.sh 20 1 
./iq-capture.sh ./iqdata.bin 300

# Select granita channels
./rx-chan.sh 0
./rx-chan.sh 1

# get vspa stats
iq_streamer -m

#clear stats
 ./stats.sh -1

# get vspa trace
iq_streamer -d

#################################################################
## 122.88 MSPS TX is achievable with VSPA DMA only in half duplex  
# no change on iq-capture.sh, but one change on iq-reaplay.sh which
# need to pass additional parameter to ask half duplex mode 
# It will use 4x VSPA DMA and 2x VSPA DMA to increase xfer throuput 
# But it is no more possible to do both TX and RX at the same time

./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200 1
./iq-capture.sh ./iqdata.bin 300 1
./iq-capture-ddr.sh 300 1

###################################################################
## 1T2R and 1T4R with decimation  
#  user need to replace default image with the multi channel images

/lib/firmaware/apm-iqplayer.eld -> /lib/firmware/apm-iqplayer-2R.eld
/usr/bin/iq_streamer -> /usr/bin/iq_streamer_2R


########################################################################
## Advanced use cases 1 : “How to connect upper app/stack to IQ Flow ?”
# iq_streamer can be used as Rx/Tx streaming application.  
# iq_streamer streaming feature, intends to demonstrate how to connect higher 
# app/stack/phy with VSPA firmware and IQ samples flow. 
# VSPA firmware will use a single free running buffer in memory DDR (or other memory TCM/OCRAM), 
# same script/config as “basic enablement” are used. The buffers will play role of intermediate sizeable TX/RX FIFO between app and VSPA firmware. 
# The streaming application will fill/consume data on the go. Handshake for now is based on VSPA exported symbols TX_total_consumed_size or RX_total_produced_size. 
# There will be 2 modes in iq_streamer, streaming_ddr and streaming_tcm, where intermediate FIFO is either on host DDR or on EP TCM memory, 
# same principle but require different DMAs. As of Today only “tx_streaming_tcm” is provided. 
# The example is streaming a large input file loaded in host DDR, streaming it into Tx FIFO in LA9310 TCM. 
# At startup, user provide address size for the large file and the tcm fifo. User can replace the large file in DDR by output of an upper stack.

taskset 0x8 iq_streamer -t -a 0x96400000 4915200 -T 0x20001000 32768 &
./iq-replay-streamer.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

# get iq_streamer trace
kill -USR1 <iq_streamer PID>

#############################################
## Advanced use cases 2 : “How to support higher sampling rate 160MSPS ?”
# iq_streamer can be used as external dma helper agent to replace VSPA DMAs with external DMA engines.  
# iq_streamer external dma agent is used to perform DMEM firmware fifo transfer using external DMA such as imx PCI DMA instead of VSPA DMAs. 
# This turns to be useful to overcome imx8 pci limitations in current RFNM firmware. 
# Write bandwidth from PCI EP into Imx8 DDR as known limitation in imx8 PCI block. 
# Throughput is higher if you read from imx8 instead of writing from EP. 
# Therefore it could be useful to use an agent running imx to perform DMA read from DMEM and write into DDR, instead of VSPA DMA writing directly DDR. 
# This agent, has strong real time constraint, because it needs to keep pace with the small DMEM FIFO. As of Today, this turns to be overkill. 
# Despite i.mx limitation, VSPA DMA can sustain 530MB/s DDR write which is enough to support 122.88MSPS. External DMA may reach PCI line rate at 800MB/s, which may be needed only to support 160MSPS. 
# iq_streamer supports this mode but can only sustain 375MB/s when running in linux user space with isolcu/noHz, this would need to move to i.mx8 M7 core to reach max throughput. Proven to work in RFNM firmware.   
# The way to start this “ext DMA” mode is to start the agent by doing “taskset 0x8 iq_streamer -t &” or/and “taskset 0x4 iq_streamer -r &”. 
# The agents will update VSPA shared flag RX/TX_ext_dma_enabled, indicating VSPA to skip VSPA DMA and let agent to managed DMEM FIFOs.

taskset 0x4 iq_streamer -r &
./iq-capture-ddr.sh 1200

taskset 0x8 iq_streamer -t &
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

taskset 0x8 iq_streamer -t -a 0x96400000 4915200 -T 0x20001000 32768 &
./iq-replay-streamer.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

# get iq_streamer trace
kill -USR1 <iq_streamer PID>

#############################################
## Know issues, limitations 

 a)	Current firmware support 61.44MSPS full duplex and only 122.88MSPS half duplex
  
              
#############################################
## Next steps :

 •	Debug sporadic Tx Fifo overrun in FD 61.44MSPS
 •	Add multi channel Rx , decimation x2 , x4
 •	Add Rx ddr streaming
 •	Enhance buffer handshake with meta data buffer descriptor instead of share variables
 •	Look at possible FD 122.88MSPS

##########################
### debug tips and tricks 

memtest , check mmap() DDR region perf, caching and coherency
-----------------------------------------------------------
This code can be used on Rx buffer while VSPA is streaming samples, 
this should print trace of captured data changes on exit when used with "i"
and should not if used without "-i" (invalidate cahe data) 

 ./load-nlm.sh
 ./iq-capture-ddr.sh 300
  running until ./iq-stop.sh and
  bin2mem -f iqdata.bin -a 0x9CC00000 -r 1228800
 ./memtest -a 0x9CC00000 -s 2097152 -i

imx_dma
-------
This tool exercise imx pci DMA

./imx_dma -w -a 0x96400000 -d 0x1F400000 -s 16384
./imx_dma -r -a 0x1F400000 -d 0x96400000 -s 16384


iq_streamer DMA agents requires hard real time , hence proper isolcpu
----------------------------------------------------------------------
 minicom -w -D /dev/ttyUSB0 -b 115200
 
 need isolcpu 3/4
 root@imx8mp-rfnm:~# dmesg |grep iso
 [    0.000000] Kernel command line: console=ttymxc1,115200 root=/dev/mmcblk1p2 rootwait rw isolcpus=2-3 nohz_full=2-3 irqaffinity=0 cpuidle.off=1 cpufreq.off=1
 root@imx8mp-rfnm:~# zcat /proc/config.gz |grep NO_HZ
 CONFIG_NO_HZ_FULL=y
 root@imx8mp-rfnm:~# zcat /proc/config.gz |grep RCU_NOCB_CPU
 CONFIG_RCU_NOCB_CPU=y

iMX PCI DMA registers
---------------------
 dma rx
  memtool -32  0x33b80300 10
 dma tx
  memtool -32  0x33b80200 10
 
check ADC/DAC clock
-------------------
  memtool -32 0x19040300 1
  00010303 -> 61.44Mhz
  00000000 -> 122.88Mhz

Steps to Generate vspa_exported_symbols.h vspa_trace_enum.h from apm-iqplayer.eld
---------------------------------------------------------------------------------
cd iq_streamer
./vspa_symbols_extract.sh ../fw_iqplayer/apm-iqplayer.eld 

copy the include/l1-trace.h to iq_streamer direcory
./vspa_trace_code_extract.sh


