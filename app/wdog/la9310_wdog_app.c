// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2021-2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>
#include <la9310_wdog_api.h>


/* Every modem can have one watchdog ID */
#define MODEM_WDOG_INSTANCE    0

#define MAX_MODEM_INSTANCES    1

#define LA9310_WDOG_SCHED_PRIORITY 98

static int modem_id;
static int wdog_event_flag;

void print_usage_message(void)
{
	printf("Usage : ./la9310_wdog_testapp [options] modem_id\n");
	printf("\n");
	printf("mandatory:\n");
	printf("\toptions: -f (or) -w\n");
	printf("\t\t-f : Force Reset\n");
	printf("\n");
	printf("\t\t-w : Watchdog Reset\n");
	printf("\n");
	printf("\tmodem_id         -- decimal | 0..(MAX_MODEM_INSTANCES-1)\n");
	printf("\nMAX_MODEM_INSTANCES=%d\n", MAX_MODEM_INSTANCES);
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	if (argc == 1)
		print_usage_message();

	if ((strcmp(argv[1], "-h") == 0)  || (strcmp(argv[1], "-H") == 0))
		print_usage_message();

	if (argc == 3) {
		modem_id  = atoi(argv[2]);
		if ((strcmp(argv[1], "-f") == 0)  ||
		    (strcmp(argv[1], "-F") == 0)) {
			wdog_event_flag = 1;
		} else if ((strcmp(argv[1], "-w") == 0)  ||
			   (strcmp(argv[1], "-W") == 0)) {
			wdog_event_flag = 0;
		} else {
			print_usage_message();
		}
	} else {
		print_usage_message();
	}
}

struct sched_param param = { .sched_priority = LA9310_WDOG_SCHED_PRIORITY };
int main(int argc, char *argv[])
{
	struct wdog wdog_t;
	int ret = 0;

	/* Validate CLI Args */
	validate_cli_args(argc, argv);
	/* Raise app priority to RT */
	/*sched_setscheduler(current, SCHED_FIFO, &param);*/
	sched_setscheduler(0, SCHED_FIFO, &param);

	ret = libwdog_open(&wdog_t, modem_id);
	if (ret < 0) {
		printf("Watchdog open failed: error: %d, modem_id: %d\n", ret, modem_id);
		return ret;
	}

	if (wdog_event_flag != 0) {
		ret = libwdog_reinit_modem_rfnm(&wdog_t, 300);
		if (ret < 0)
			printf("modem reinit failed\n");
	} else {
		/* Register modem irq through ioctl */
		ret = libwdog_register(&wdog_t);
		if (ret < 0) {
			printf("wdog register failed with error: %d\n", ret);
			return ret;
		}

		libwdog_wait(wdog_t.dev_wdog_handle);
		libwdog_deregister(&wdog_t);

		ret = libwdog_reinit_modem_rfnm(&wdog_t, 300);
		if (ret < 0)
			printf("WDOG Event: modem reinit failed\n");
		else
			return 0;
	}

	libwdog_close(&wdog_t);

	return 0;
}
