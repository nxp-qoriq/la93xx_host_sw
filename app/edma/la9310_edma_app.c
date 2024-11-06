// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <la9310_modinfo.h>
#include <imx_edma_api.h>

int ch_num = 1;
int type;
uint32_t src_addr, dst_addr, sz;
uint32_t edma_base_addr = IMX8MP_DMA_BASE_ADDR;
uint32_t edma_reg_size = IMX8MP_DMA_REG_SPACE_SIZE;

void print_usage_message(void)
{
	printf("Usage : ./la9310_edma_app -c [val] -s [val] -d [val] -l [val] [option] \n");
	printf("\n");
	printf("options: -r (read), -w (write), by default it is write\n");
	printf("\t -c: Current channel number - default 1\n");
	printf("\t -b: PCI EDMA base address\n");
	printf("\t -n: PCI EDMA	reg space size\n");
	printf("\t -s: Source Address : default: 0x96400000(for write)\n");
	printf("\t -d: Destination Address: default: 0x1f000000(for write)\n");
	printf("\t -l: Tansfer Len - default 0x20\n");
	printf("\t Read-> Dma transfer from Ep to RC\n");
	printf("\t Write-> Dma transfer from Ep to RC\n");
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	int opt;
	char *endp = NULL;

	while ((opt = getopt(argc, argv, "c:b:n:s:d:l:rwh")) != -1) {
		switch (opt) {
		case 'c':
			ch_num = atoi(optarg);
			break;
		case 'b':
			edma_base_addr = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'n':
			edma_reg_size = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 's':
			src_addr = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'd':
			dst_addr = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'l':
			sz = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'r':
			type = 1;
			break;
		case 'w':
			type = 0;
			break;
		default:
			print_usage_message();
			break;
		}
		endp = NULL;
	}
}

void dma_init(void)
{
	int fd;
	modinfo_t mil = {0};
	modinfo_t *mi = &mil;
	int ret, i;
	char dev_name[32];

	sprintf(dev_name, "/dev/%s%d", LA9310_DEV_NAME_PREFIX, 0);

	fd = open(dev_name, O_RDWR);
	if (fd < 0) {
		printf("File %s open error\n", dev_name);
		return;
	}

	ret = ioctl(fd, IOCTL_LA93XX_MODINFO_GET, (modinfo_t *) &mil);
	if (ret < 0) {
		printf("IOCTL_LA9310_MODINFO_GET failed.\n");
		close(fd);
		return;
	}

	src_addr = mi->iqflood.host_phy_addr;
	dst_addr = mi->tcmu.host_phy_addr;

	close(fd);
}

int main(int argc, char *argv[])
{
	uint64_t rd_pw_en_reg, wr_pw_en_reg;

	dma_init();
	validate_cli_args(argc, argv);

	init_mem(edma_base_addr, edma_reg_size);

	if (!type)
		pci_dma_write(src_addr, dst_addr, sz, ch_num);
	else
		pci_dma_read(dst_addr, src_addr, sz, ch_num);

	deinit_mem(edma_reg_size);

	return 0;
}
