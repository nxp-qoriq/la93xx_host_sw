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
./enable_rf.sh granitatx
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 1200

# capture (one time) iq samples into file
./load-nlm.sh
./enable_rf.sh granitarx
./iq-capture.sh ./iqdata.bin 300

# capture (repeat) in DDR buffer 
./load-nlm.sh
./enable_rf.sh granitarx
./iq-capture-ddr.sh 300
./iq-stop.sh
bin2mem -f 300 -a 0x96400000 -r 1228800

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
./iq_streamer/iq_streamer -m

#clear stats
 ./stats.sh -1

# get vspa trace
./iq_streamer/iq_streamer -d

#################################################################
## 122.88 MSPS TX is achievable with VSPA DMA only in half duplex  

./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 1200 1


#############################################
## 122.88+ MSPS full duplex requires imx DMA 

taskset 0x8 ./iq_streamer/iq_streamer -r &
taskset 0x8 ./iq_streamer/iq_streamer -t &
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB1200_signM.bin 1200
./iq-capture-ddr.sh 300

# get iq_streamer trace
kill -USR1 <iq_streamer PID>

##########################
### debug tips and tricks 

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

