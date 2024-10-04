###################################################
# singleTone play/capture
#
# scripts are compatible with apm-iqplayer.eld
# File size is num of 4KB buffer
###################################################

#######################################################
## Standard perf using , standalone vspa using VSPA DMA

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
./rx-chan.sh 4
./rx-chan.sh 2

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
# It will use 4x VSPA DMA to increase DDR read throughput but impact
# the rx path. So it is no more possible to do boath TX and RX at the same time

./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200 1


#############################################
## experimental external imx DMA agent 

# This intend to support 160MSP which requires 640MB/s
# hence VSPA DMA with 590MB/s is too short
# This code use imx pci DMA instead of VSPA DMA
# in RX iq_streamer will do DMA from dmem to DDR
# in TX iq_streamer will do DMA from DDR to TCM and VSPA DMA from TCM to DMEM
# This is example code which only works at 61.44MSPS due to real time limit of
# userspace app.. such approach is proven to work in RX in M7 core (RFNM)

taskset 0x4 iq_streamer -r &
./iq-capture-ddr.sh 300

taskset 0x8 iq_streamer -t &
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

taskset 0x8 iq_streamer -t -a 0x96400000 4915200 -T 0x20001000 32768 &
./iq-replay-streamer.sh ./tone_td_3p072Mhz_20ms_4KB1200_2c.bin 1200

# get iq_streamer trace
kill -USR1 <iq_streamer PID>

##########################
### debug tips and tricks 

memtest , check mmap() DDR region perf, caching and coherency
-----------------------------------------------------------
This code can be used on Rx buffer while VSPA is streaming samples, 
this should print trace of captured data changes on exit when used with "i"
and should not if used without "-i" (invalidate cahe data) 

# ./load-nlm.sh
# ./iq-capture-ddr.sh 300
  running until ./iq-stop.sh and
  bin2mem -f iqdata.bin -a 0x9CC00000 -r 1228800
# ./memtest -a 0x9CC00000 -s 2097152 -i

imx_dma
-------
This tool exercise imx pci DMA

./imx_dma -w -a 0x96400000 -d 0x1F400000 -s 16384
./imx_dma -r -a 0x1F400000 -d 0x96400000 -s 16384


iq_streamer DMA agents requires hard real time , hence proper isolcpu
----------------------------------------------------------------------
 need isolcpu 3/4
 root@imx8mp-rfnm:~# dmesg |grep iso
 [    0.000000] Kernel command line: console=ttymxc1,115200 root=/dev/mmcblk1p2 rootwait rw isolcpus=2-3 nohz_full=2\x1b2-3 irqaffinity=0 cpuidle.off=1 cpufreq.off=1
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
---------------------------------------------------------------
cd iq_streamer
./vspa_symbols_extract.sh ../fw_iqplayer/apm-iqplayer.eld 

copy the include/l1-trace.h to iq_streamer direcory
./vspa_trace_code_extract.sh


