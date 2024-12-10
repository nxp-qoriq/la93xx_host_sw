// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define FRE_CORRECTION_CMD_STR  "/sys/devices/platform/soc@0/30800000.bus/30a20000.i2c/i2c-0/0-0058/si5510_freq_correction"

int main(int argc, char *argv[])
{

	unsigned long fre_correction = 0;
	unsigned char clock_div = 0;
	int opt;
	int operation = 0;
	char cmd_str[256];
	if(argc < 7)
		goto out;

	while ((opt = getopt (argc, argv, "o:d:c:h")) != -1) {
		switch (opt) {
		    case 'o':
			if(!strncmp(optarg,"add",3))
			    operation = 1;
			else if (!strncmp(optarg,"sub",3))
			    operation = 2;
			break;
		    case 'd':
			clock_div = (strtoul(optarg, NULL, 0) & 0xff );
			break;
		    case 'c':
			fre_correction = strtoul(optarg, NULL, 0);
			break;
		    default :
			goto out;
		}
	}
	if(!operation) {
		goto out;
	}
	else {
		if(operation == 1)  //Add operation
			sprintf(cmd_str, "echo 0x%02x%08x > %s",clock_div,(unsigned int)fre_correction,FRE_CORRECTION_CMD_STR);
		else  //Sub operation
			sprintf(cmd_str, "echo 0x%02x%08x > %s",clock_div,(unsigned int)~fre_correction,FRE_CORRECTION_CMD_STR);

		system(cmd_str);
	}

	return 0;

	out:
		printf("Valid option --> \r\n %s -o <add/sub> -d <clk_div 1/2/8>  -c <frequency correction> \r\n",argv[0]);
		return 1;
}
