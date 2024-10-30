#!/bin/sh
# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2024 NXP
####################################################################
#nxp@nxp-Precision-5820-Tower:~/Freescale/tmp/VSPA_Tools/bin$ ./vspa-elfdump -y /home/nxp/apm.eld |grep RX_total_produced_size
#    0x000033f8  0x00000000    LOCAL       NOTYPE         NONE       17  aiqmod_rx_ab_8__RX_total_produced_size
#    0x000072a0  0x00000004   GLOBAL       OBJECT     VARIABLE        1  _RX_total_produced_size
#set -x
echo "### exporting vspa trace code from l1-trace.h"
eld_md5sum=$(md5sum  ../../Debug_IQPLAYER/apm-iqplayer.eld)

echo "/* SPDX-License-Identifier: BSD-3-Clause" > ./vspa_trace_enum.h 
echo "* Copyright 2024 NXP">> ./vspa_trace_enum.h 
echo " */">> ./vspa_trace_enum.h 


echo "/* File generated based on apm.eld md5sum" $eld_md5sum "*/" >> ./vspa_trace_enum.h 
echo "l1_trace_code_t l1_trace_code[] ={" >> ./vspa_trace_enum.h  

dos2unix ../../include/l1-trace.h
grep " L1_TRACE_" ../../include/l1-trace.h  | grep "\*" > ./vspa_trace_enum_0.txt
sed 's/\*//g' vspa_trace_enum_0.txt > ./vspa_trace_enum_1.txt
sed 's/\///g' vspa_trace_enum_1.txt > ./vspa_trace_enum_2.txt
sed 's/,/ /g' vspa_trace_enum_2.txt > ./vspa_trace_enum.txt

while read in; do
	code=$(echo $in | cut -f 2  -d "*" | cut -f 1 -d " ")
	text=$(echo $in | cut -f 3  -d "*"| cut -f 2 -d " ")
	echo "{ "$code", \""$text "\"}," >> ./vspa_trace_enum.h  
done < ./vspa_trace_enum.txt
	echo "{ 0xFFFF, \"MAX_CODE_TRACE\"}" >> ./vspa_trace_enum.h  

echo "};" >> ./vspa_trace_enum.h 

rm vspa_trace_enum*.txt  

