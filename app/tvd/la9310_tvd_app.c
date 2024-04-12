/*  SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 *  Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sched.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "la9310_tvd_ioctl.h"

#ifndef MAX_MODEM
#define MAX_MODEM 4
#endif
#define MAX_EVENTS 1
#define	EINVAL		22	/* Invalid argument */
#ifdef DEBUG_THERMAL
#define pr_debug(...) printf(__VA_ARGS__)
#else
#define pr_debug(...)
#endif
#define LA9310_DEV_NAME_PREFIX	"shiva"

static int modem_id;
static bool modem_monitor;
static bool cpu_monitor;

void print_usage_message(void)
{
	printf("Usage: ./la9310_tvd_testapp modem_id -m -c\n");
	printf("       OR\n");

	printf("\tmodem_id    -- decimal | 0..(%d)\n", MAX_MODEM-1);
	printf("Options:\n");
	printf("\t–h\tHelp\n");
	printf("\t–m\tMonitor Modem temperature\n");
	printf("\t–c\tMonitor Host temperature\n");
	fflush(stdout);
	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	int opt;

	if (argc <= 1)
		print_usage_message();

	modem_id  = atoi(argv[1]);
	while ((opt = getopt(argc, argv, "mch")) != -1) {
		switch (opt) {
		case 'm':
			modem_monitor = true;
			break;
		case 'c':
			cpu_monitor = true;
			break;
		case '?':
			printf("Unknown option: -%c\n", optopt);
			print_usage_message();
			break;
		default:
			print_usage_message();
			break;
		}
	}
}

void print_tvd_info(int id)
{
	int fd;
	int ret;
	char dev_name[32];
	struct tvd tvd_t = {0};
	struct tvd *tvd_ptr = &tvd_t;

	sprintf(dev_name, "/dev/%s%s%d",
		LA9310_DEV_NAME_PREFIX, LA9310_TVD_DEV_NAME_PREFIX, id);

	fd = open(dev_name, O_RDWR);
	if (fd < 0) {
		printf("File %s open error\n", dev_name);
		return;
	}

	if (modem_monitor) {
		ret = ioctl(fd, IOCTL_LA9310_TVD_MTD_GET_TEMP, (struct tvd *) &tvd_t);
		if (ret < 0) {
			printf("IOCTL_LA9310_TVD_MTD_GET_TEMP failed.\n");
			close(fd);
			return;
		}

		printf("Modem VSPA current temp: %d\n",
				tvd_ptr->get_mtd_curr_temp.vspa_temp);
		printf("Modem DCS current temp: %d\n",
				tvd_ptr->get_mtd_curr_temp.dcs_temp);
	}

	if (cpu_monitor) {
		ret = ioctl(fd, IOCTL_LA9310_TVD_CTD_GET_TEMP, (struct tvd *) &tvd_t);
		if (ret < 0) {
			printf("IOCTL_LA9310_TVD_CTD_GET_TEMP failed.\n");
			close(fd);
			return;
		}

		printf("Host probe1 current temp: %d\n", tvd_ptr->get_ctd_curr_temp);
	}

	close(fd);
}

int main(int argc, char *argv[])
{
	int i = 0;
	validate_cli_args(argc, argv);

	if (modem_id != -1)
		print_tvd_info(modem_id);
	else {
		for (i = 0; i < MAX_MODEM; i++)
			print_tvd_info(i);
	}
}
