###################################################
# singleTone play/capture
#
# Script compatible with VSPA firmware apm-nlm.eld
# Script compatible with M4 firmware la9310_dfe.bin
# 

# play single tone on granita
./load-nlm.sh
./enable_rf.sh granitatx
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 300
./stats.sh 0

# capture on granita 2 channels
./load-nlm.sh
./enable_rf.sh granita2R
./rx-chan.sh 4
./iq-capture.sh ./iqdata4.bin 30
./rx-chan.sh 2
./iq-capture.sh ./iqdata2.bin 30

# play and capture single tone on Granita
./load-nlm.sh
./enable_rf.sh granitatxrx
./singleTone.sh 20 1 
./rx-chan.sh 2
./iq-capture.sh ./iqdata.bin 30

# vspa sanity test axiq loop back testing
./load-nlm.sh
mount -t hugetlbfs none /dev/hugepages
echo 24 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
dpdk-dfe_app -c "axiq_lb enable"
./singleTone.sh 20 1 
./iq-capture.sh ./iqdata.bin 30

