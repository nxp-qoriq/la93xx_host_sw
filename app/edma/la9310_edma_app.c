// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <la9310_modinfo.h>
#include <imx_edma_api.h>

int ch_num = 1;
int dir, sg_en = 0, nb_desc = 0;
uint32_t host_addr, mdm_addr, sz = DEFAULT_TXRX_SIZE;
uint32_t edma_base_addr = IMX8MP_DMA_BASE_ADDR;
uint32_t edma_reg_size = IMX8MP_DMA_REG_SPACE_SIZE;
uint32_t ll_base_addr = LL_TABLE_ADDR;

void print_usage_message(void)
{
	printf("Usage : ./la9310_edma_app -c [val] -s [val] -d [val] -l [val] [option] \n");
	printf("\n");
	printf("options: -r (read), -w (write), by default it is write\n");
	printf("\t -c: Current channel number - default 1\n");
	printf("\t -b: PCI EDMA base address\n");
	printf("\t -n: PCI EDMA	reg space size\n");
	printf("\t -h: Host Address : default: 0x96400000(for write)\n");
	printf("\t -m: Modem Address: default: 0x1f000000(for write)\n");
	printf("\t -s: Tansfer Len - default 0x20\n");
	printf("\t Read-> Dma transfer from Ep to RC\n");
	printf("\t Write-> Dma transfer from RC to EP\n");
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	int opt;
	char *endp = NULL;

	while ((opt = getopt(argc, argv, "c:b:l:h:m:s:t:n:rwh")) != -1) {
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
		case 'l':
			edma_reg_size = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'h':
			host_addr = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 'm':
			mdm_addr = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 's':
			sz = strtoull(optarg, &endp, 0);
			if ((endp && 0 != *endp)) {
				printf("Invalid memory address: %s\n", optarg);
				print_usage_message();
				exit(2);
			}
			break;
		case 't':
			sg_en = atoi(optarg);
			break;
		case 'n':
			nb_desc = atoi(optarg);
			break;
		case 'r':
			dir = 1;
			break;
		case 'w':
			dir = 0;
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

	host_addr = mi->iqflood.host_phy_addr;
	mdm_addr = mi->tcmu.host_phy_addr;

	close(fd);
}

int main(int argc, char *argv[])
{
	uint64_t rd_pw_en_reg, wr_pw_en_reg;
	uint32_t s_addr, d_addr, dma_status;
	host_desc_t *h_desc;
	mdm_desc_t *m_desc;

	dma_init();
	validate_cli_args(argc, argv);

	pci_dma_mem_init(edma_base_addr, edma_reg_size, ll_base_addr, nb_desc, sg_en);

	if (sg_en) {
		h_desc = malloc(sizeof(h_desc) * nb_desc);
		m_desc = malloc(sizeof(m_desc) * nb_desc);

		for (int i=0; i<nb_desc; i++) {
			h_desc[i].addr = host_addr + sz*i;
			m_desc[i].addr = mdm_addr + sz*i;
			h_desc[i].size = sz;
			m_desc[i].size = sz;
		}

		pci_dma_sg_configure(h_desc, m_desc, nb_desc, dir);
		s_addr = ll_base_addr;
		d_addr = -1;
	} else {
		if (!dir) {
			s_addr = host_addr;
			d_addr = mdm_addr;
		} else {
			s_addr = mdm_addr;
			d_addr = host_addr;
		}

	}
	if (!dir)
		pci_dma_write(s_addr, d_addr, sz, ch_num, sg_en);
	else
		pci_dma_read(s_addr, d_addr, sz, ch_num, sg_en);

	do {
		dma_status = pci_dma_completed(ch_num, dir);

		if (dma_status == -2) {
			printf("Dma Halted due to some error\n");
			break;
		}
	} while(dma_status);
	pci_dma_mem_deinit(edma_reg_size, nb_desc);

	return 0;
}
