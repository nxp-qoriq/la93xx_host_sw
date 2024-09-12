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
./iq-capture.sh ./iqdata.bin 30

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
./iq-capture.sh ./iqdata.bin 30

# Select granita channels
./rx-chan.sh 4
./rx-chan.sh 2

# get vspa stats
./iq-streamer/iq_streamer -m
for i in {0..11} ; do ./stats.sh $i;done | grep MSB

#clear stats
 ./stats.sh -1

#######################################################
## Max perf using host DMA helper agents "iq_streamer" 

# All same scipts/command, just need to start DMA helper agents "iq_streamer -r/-t" 

ex: 
./iq-stop.sh
taskset 0x4 ./iq-streamer/iq_streamer -r &
./iq-capture.sh ./iqdata2.bin 30
./iq-capture-ddr.sh 30

taskset 0x8 ./iq-streamer/iq_streamer -t &
taskset 0x4 ./iq-streamer/iq_streamer -r &
./iq-capture-ddr.sh 30


# get iq-streamer stats
kill -USR1 <iq-streamer PID>


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
