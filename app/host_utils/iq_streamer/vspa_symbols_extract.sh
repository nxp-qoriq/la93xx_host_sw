#!/bin/bash
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#nxp@nxp-Precision-5820-Tower:~/Freescale/tmp/VSPA_Tools/bin$ ./vspa-elfdump -y /home/nxp/apm.eld |grep RX_total_produced_size
#    0x000033f8  0x00000000    LOCAL       NOTYPE         NONE       17  aiqmod_rx_ab_8__RX_total_produced_size
#    0x000072a0  0x00000004   GLOBAL       OBJECT     VARIABLE        1  _RX_total_produced_size
#set -x
echo "### exporting vspa symbols from apm.eld"
grep VSPA_EXPORT *.c|cut -f 2 -d "(" |cut -f 1 -d ")"> ./vspa_exported_symbols0.txt
grep VSPA_EXPORT ../common/*.c|cut -f 2 -d "(" |cut -f 1 -d ")">> ./vspa_exported_symbols0.txt
sed 's/\r$//' vspa_exported_symbols0.txt > ./vspa_exported_symbols.txt
/home/nxp/Freescale/tmp/VSPA_Tools/bin/vspa-elfdump -y $1 >./$1.symbol.txt
while read in; do
	grep "$in" ./$1.symbol.txt|grep GLOBAL >> ./$1_vspa_exported_symbols_addr.txt
done < ./vspa_exported_symbols.txt

echo "/* SPDX-License-Identifier: BSD-3-Clause" > $2
echo "* Copyright 2024 NXP">> $2
echo " */">> $2


eld_md5sum=$(md5sum ./$1 | cut -f 1 -d " ")
echo "#define IQ_STREAMER_VERSION \""$eld_md5sum"\"" >> $2

md5sum ./$1

while read in; do
	symbol=$(echo $in |  cut -f 7 -d ' ')
	addr=$(echo $in |   cut -f 1 -d ' ')
	size=$(echo $in |   cut -f 2 -d ' ')
	if [[ $addr -ge 0x4000 ]]; then
		start=0x500000;
	else
 		start=0x400000;
	fi
	if [ $# -gt 2 ];then
	echo "#define " v$symbol "(volatile uint32_t *)((uint32_t)BAR2_addr + "$start" + "$addr")" >> $2
	else
	echo "#define " v$symbol "(volatile uint32_t *)((uint64_t)BAR2_addr + "$start" + "$addr")" >> $2
	fi
	echo "#define " p$symbol "(uint32_t)(0x1F000000 + "$start" + "$addr")" >> $2
	echo "#define " s$symbol "(uint32_t)("$size")" >> $2
done < ./$1_vspa_exported_symbols_addr.txt

rm ./vspa_exported_symbols0.txt ./vspa_exported_symbols.txt ./$1.symbol.txt ./$1_vspa_exported_symbols_addr.txt
