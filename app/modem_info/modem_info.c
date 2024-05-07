// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <la9310_modinfo.h>

int modem_id = 0;
int stats;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

void print_usage_message(void)
{
	printf("Usage : ./modem_info [optional]\n");
	printf("\n");
	printf("\t-h or -H    help and usages\n");
	printf("\t-m <dev id>\n");
	printf("\t-s  got printing stats\n");
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	int strsize, opt;

	if (argc < 2)
		return;

	while ((opt = getopt(argc, argv, ":m:sh")) != -1) {
		switch (opt) {
		case 'm':
			modem_id = strtoul(optarg, NULL, 0);
			break;
		case 's':
			stats = 1;
			break;
		case 'h':
			print_usage_message();
			break;
		case '?':
			/*  FALLTHROUGH */
		default:
			print_usage_message();
			break;
		}
	}
}

void print_modem_stats(int fd)
{
	modinfo_s mil = {0};
	modinfo_s *mi = &mil;
	int ret;

	ret = ioctl(fd, IOCTL_LA93XX_MODINFO_GET_STATS, (modinfo_s *) &mil);
	if (ret < 0) {
		printf("IOCTL_LA9310_MODINFO_GET failed.\n");
		close(fd);
		return;
	}

	printf("\nLA9310 host stats dump\n");
	printf("%s", mi->target_stat);
}

void print_modem_info(int id)
{
	int fd;
	modinfo_t mil = {0};
	modinfo_t *mi = &mil;
	int ret, i;
	char dev_name[32];

	sprintf(dev_name, "/dev/%s%d", LA9310_DEV_NAME_PREFIX, id);

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

	printf("Board Name - %s\n", mi->board_name);
	printf("LA93xx ID:%d\n", mi->id);
	printf("Dev name - %s\n", mi->name);
	printf("PCI addr - %s\n", mi->pci_addr);
	printf("PCI WIN start     0x%lx Size:0x%x\n",
		mi->pciwin.host_phy_addr, mi->pciwin.size);
	printf("HIF start         0x%lx Size:0x%x\n",
		mi->hif.host_phy_addr, mi->hif.size);
	printf("CCSR phys         0x%lx Size:0x%x\n",
		mi->ccsr.host_phy_addr, mi->ccsr.size);
	printf("TCML phys         0x%lx Size:0x%x\n",
		mi->tcml.host_phy_addr, mi->tcml.size);
	printf("TCMU phys         0x%lx Size:0x%x\n",
		mi->tcmu.host_phy_addr, mi->tcmu.size);
	printf("VSPA OVERLAY phys 0x%lx Size:0x%x\n",
		mi->ov.host_phy_addr, mi->ov.size);
	printf("VSPA start        0x%lx Size:0x%x\n",
		mi->vspa.host_phy_addr, mi->vspa.size);
	printf("FW start          0x%lx Size:0x%x\n",
		mi->fw.host_phy_addr, mi->fw.size);
	printf("DBG LOG phys      0x%lx Size:0x%x\n",
		mi->dbg.host_phy_addr, mi->dbg.size);
	printf("IQ SAMPLES phys   0x%lx Size:0x%x\n",
		mi->iqr.host_phy_addr, mi->iqr.size);
	printf("IQ FLOOD phys     0x%lx	Modem phys:0x%x Size:0x%x\n",
		mi->iqflood.host_phy_addr, mi->iqflood.modem_phy_addr, mi->iqflood.size);
	printf("NLM OPS start     0x%lx Size:0x%x\n",
		mi->nlmops.host_phy_addr, mi->nlmops.size);
	printf("STD FW phys       0x%lx Size:0x%x\n",
		mi->stdfw.host_phy_addr, mi->stdfw.size);
	printf("Scratch buf phys  0x%lx Size:0x%x\n",
		mi->scratchbuf.host_phy_addr, mi->scratchbuf.size);
	printf("DAC Mask: 0x%x  Rate: %s\n",
			mi->dac_mask,
			mi->dac_rate_mask ? "61.44 MHz":"122.88 MHz");
	for (i = 0; i < 4; i++) {
		printf("ADC-%d: %s  Rate : %s \n",
			i, CHECK_BIT(mi->adc_mask,i) ? "ON": "OFF",
			CHECK_BIT(mi->adc_rate_mask,i) ? "61.44 MHz":"122.88 MHz");
	}

	if (stats == 1)
		print_modem_stats(fd);
	close(fd);
}

int main(int argc, char *argv[])
{
	int i = 0;

	/* Validate CLI Args */
	validate_cli_args(argc, argv);

	print_modem_info(modem_id);
}
