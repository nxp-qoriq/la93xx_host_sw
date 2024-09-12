#!/bin/bash
# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
#nxp@nxp-Precision-5820-Tower:~/Freescale/tmp/VSPA_Tools/bin$ ./vspa-elfdump -y /home/nxp/apm.eld |grep RX_total_produced_size
#    0x000033f8  0x00000000    LOCAL       NOTYPE         NONE       17  aiqmod_rx_ab_8__RX_total_produced_size
#    0x000072a0  0x00000004   GLOBAL       OBJECT     VARIABLE        1  _RX_total_produced_size
#set -x
echo "### exporting vspa symbols from apm.eld"
grep VSPA_EXPORT *.c|cut -f 2 -d "(" |cut -f 1 -d ")"> ./vspa_exported_symbols0.txt
sed 's/\r$//' vspa_exported_symbols0.txt > ./vspa_exported_symbols.txt
/home/nxp/Freescale/tmp/VSPA_Tools/bin/vspa-elfdump -y $1 >./$1.symbol.txt 
while read in; do
	grep "$in" ./$1.symbol.txt|grep GLOBAL >> ./$1_vspa_exported_symbols_addr.txt
done < ./vspa_exported_symbols.txt

eld_md5sum=$(md5sum ./$1)

echo "/* SPDX-License-Identifier: BSD-3-Clause" > ./vspa_exported_symbols.h 
echo " * Copyright 2024 NXP" > ./vspa_exported_symbols.h 
echo "*/" > ./vspa_exported_symbols.h 

echo "/* File generated based on apm.eld md5sum" $eld_md5sum "*/" > ./vspa_exported_symbols.h 

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
	echo "#define " v$symbol "(volatile uint32_t *)((uint64_t)BAR2_addr + " $start " + " $addr ")" >> ./vspa_exported_symbols.h
	echo "#define " p$symbol "(uint32_t)(0x1F000000 + " $start " + " $addr ")" >> ./vspa_exported_symbols.h
	echo "#define " s$symbol "(uint32_t)(" $size ")" >> ./vspa_exported_symbols.h
done < ./$1_vspa_exported_symbols_addr.txt

rm ./vspa_exported_symbols0.txt ./vspa_exported_symbols.txt ./$1.symbol.txt ./$1_vspa_exported_symbols_addr.txt 
