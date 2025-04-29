/* SPDX-License-Identifier: BSD-3-Clause */

/*
 * Copyright 2025 NXP
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <inttypes.h>
 #include <string.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/ioctl.h>
 #include <sys/timerfd.h>
 #include <sys/mman.h>
 #include <errno.h>
 #include <stdbool.h>
 #include <argp.h>
 #include "HAL-imx8mp.h"
 #include "rffe.h"
 #include "SKY58281-11.h"

 #define RFFE_GPIO_PORT 3
 #define RFFE_GPIO_CLK_ENABLE() __GPIOX_CLK_ENABLE()

/*
SKY58281-11
MIPI_RFFE_RX_CLK    AE16  SAI5_RXD0 AE16   GPIO3_IO21
MIPI_RFFE_RX_DATA   AD16  SAI5_RXD1 AD16   GPIO3_IO22
MIPI_RFFE_TX_CLK    AF16  SAI5_RXD2 AF16   GPIO3_IO23
MIPI_RFFE_TX_DATA   AE14  SAI5_RXD3 AE14   GPIO3_IO24
Seeve board ext
RFFE_EXT_SCK_PIN RFNM_GPIO5_10 AH21 ECSPI2_SCLK
RFFE_EXT_SDA_PIN RFNM_GPIO5_00 AH19 SAI3_TXC  GPT1_CAPTURE1

*/

 #define RFFE_RX_SCK_PIN RFNM_GPIO3_21
 #define RFFE_RX_SDA_PIN RFNM_GPIO3_22
 #define RFFE_TX_SCK_PIN RFNM_GPIO3_23
 #define RFFE_TX_SDA_PIN RFNM_GPIO3_24
 #define RFFE_EXT_SCK_PIN RFNM_GPIO5_10
 #define RFFE_EXT_SDA_PIN RFNM_GPIO5_00

// PERIOD 40 at 35Khz -> 40/35000 = 1140µs
// RFFE standard 32Khz to 26Mhz i.e. period 36µs,38ns
 #define RFFE_PRESCALER 1
 #define RFFE_PERIOD 36000
 #define RFFE_CLK_DIV 1


void print_cmd_help(void)
{
	fprintf(stderr, "\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
	fprintf(stderr, "\n|    rffe_app -R/T/X [-a <reg addr>] [-v <reg val>][-d][-c <count>]");
	fprintf(stderr, "\n|    ex: ./rffe_app -R -a 0x1c ");
	fprintf(stderr, "\n|    ex: ./rffe_app -R -a 0x1c -v 0x38  ");
	fprintf(stderr, "\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

int main(int argc, char *argv[])
{
	int i = 0, dir =  -1, reg_addr =  -1, reg_val =  -1, count = -1;
	int32_t c;
	uint8_t outputVal[16];

	l1_trace_disable = 1;

	HAL_Init();

	/* command line parser */
	while ((c = getopt(argc, argv, "RTXwra:v:tdc:")) != EOF) {
		switch (c) {
		case 'h': // help
			print_cmd_help();
			exit(1);
		break;
		case 'R':
			dir = 0;
		break;
		case 'T':
			dir = 1;
		break;
		case 'X':
			dir = 2;
		break;
		case 'a':
			reg_addr = strtoull(argv[optind-1], 0, 0);
		break;
		case 'v':
			reg_val = strtoull(argv[optind-1], 0, 0);
			int idx = optind-1;

			i = 0;
			while (1) {
				if (argv[idx+i] == NULL)
					break;
				if (argv[idx+i][0] == '-')
					break;
				outputVal[i] = strtoull(argv[idx+i], 0, 0);
				if (++i >= 16)
					break;
			}
			if (i > 1)
				count = i;
		break;
		case 'd':
			l1_trace_disable = 0;
		break;
		case 'c':
			count = strtoull(argv[optind-1], 0, 0);
		break;
		default:
			print_cmd_help();
			exit(1);
		break;
		}
	}


	if ((-1 == reg_addr) && (-1 != reg_val)) {
		print_cmd_help();
		exit(1);
	}
	if (count > 16)
		printf("\n invalid byte count >16  (%d)\n", count);

	/* execute command */
	switch (dir) {
	case 0:
		RFFE_Init(SKY58281_RX_SLAVE_ADDR, RFFE_GPIO_PORT, RFFE_RX_SCK_PIN, RFFE_RX_SDA_PIN);
	break;
	case 1:
		RFFE_Init(SKY58281_TX_SLAVE_ADDR, RFFE_GPIO_PORT, RFFE_TX_SCK_PIN, RFFE_TX_SDA_PIN);
	break;
	case 2:
		RFFE_Init(SKY58281_TX_SLAVE_ADDR, RFFE_GPIO_PORT, RFFE_EXT_SCK_PIN, RFFE_EXT_SDA_PIN);
	break;
	default:
		printf("\n invalid direction %d\n", dir);
		print_cmd_help();
		exit(1);
	break;
	}

	// dump all
	if (-1 == reg_addr) {
		// dump all
		for (int reg_addr = 0; reg_addr <= 0x1F; reg_addr++) {
			reg_val = RFFE_ReadByte(reg_addr);
			if (reg_val)
				printf("\n 0x%02x: 0x%02x", reg_addr, reg_val);
		}
		goto end;
	}

	// single read/write
	if ((-1 == count) && (reg_addr <= 0x1F)) {
		if (-1 == reg_val) {
			//read reg
			reg_val = RFFE_ReadByte(reg_addr);
			printf(" 0x%02x: 0x%02x", reg_addr, reg_val);
		} else {
			// write reg
			RFFE_WriteByte(reg_addr, reg_val);
		}
	goto end;
	}

	if (-1 == count)
		count = 1;

	// ext read
	if (-1 == reg_val) {
		RFFE_ExtReadByte(count, reg_addr, outputVal);
		printf("reading 0x%02x: ", reg_addr);
		for (i = 0; i < count; i++)
			printf(" 0x%02x", outputVal[i]);
	goto end;
	}

	//ext write
	RFFE_ExtWriteByte(count, reg_addr, outputVal);
	printf("writing 0x%02x: ", reg_addr);
	for (i = 0; i < count; i++)
		printf(" 0x%02x", outputVal[i]);

end:
	printf("\n");
	if (!l1_trace_disable)
		print_host_trace();
	exit(1);
}
