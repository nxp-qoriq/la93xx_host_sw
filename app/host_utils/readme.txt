# install vspa_mbox
gcc vspa_mbox.c -o vspa_mbox
cp vspa_mbox /usr/bin

# play single tone on lime
./load-nlm.sh
./enable_rf.sh limetx
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 300
./stats.sh 0

# capture on granita 2 channels
./load-nlm.sh
./enable_rf.sh granita2R
./iq-capture.sh ./iqdata4.bin 30
./rx-chan.sh 2
./iq-capture.sh ./iqdata2.bin 30

# play and capture single tone on Granita
./load-nlm.sh
./enable_rf.sh granita
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 300
./rx-chan.sh 2
./iq-capture.sh ./iqdata.bin 30

# vspa sanity test axiq loop back testing
./load-nlm.sh
dpdk-dfe_app -c "axiq_lb enable"
./iq-replay.sh ./tone_td_3p072Mhz_20ms_4KB300_2c.bin 300
./iq-capture.sh ./iqdata.bin 30

