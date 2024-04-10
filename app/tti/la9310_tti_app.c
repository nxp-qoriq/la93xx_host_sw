/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright 2024 NXP
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>
#include <la9310_tti_ioctl.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>


static int modem_id;
static int tti_count;
static int tti_event_flag;

#ifndef MAX_MODEM
#define MAX_MODEM 4
#endif
/* Every modem can have one to four TTI IDs */
#define MAX_COUNT		10
#define	LA9310_RFIC_SCHED_PRIORITY	99
#define MAX_EVENTS 1

void print_usage_message(void)
{
	printf("Usage : ./app_name modem_id tti_count_max [options] \n");
	printf("\n");
	printf("options:\n");
	printf("\n-e : eventMode\n");
	printf("\n");
	printf("mandatory:\n");
	printf("\tmodem_id         -- decimal |	0..%d\n", MAX_MODEM-1);
	printf("\ttti_count_max	 -- decimal |	0..%d\n", MAX_COUNT-1);
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{

	if (argc <=1)
		print_usage_message();

	if ((strcmp(argv[1], "-h") == 0)  ||
			(strcmp(argv[1], "-H") == 0))
		print_usage_message();

	/* Check Total Argument count */
	if (argc >= 3) {
		modem_id	= atoi(argv[1]);
		tti_count	= atoi(argv[2]);
		if ((argc > 3) && ((strcmp(argv[3], "-e") == 0)  ||
			(strcmp(argv[3], "-E") == 0))) {
			tti_event_flag = 1;
		} else {
			tti_event_flag = 0;
		}
	} else {
		print_usage_message();
	}
}

struct sched_param param = { .sched_priority = LA9310_RFIC_SCHED_PRIORITY };
int main(int argc, char *argv[])
{
	struct tti tti_t;
	ssize_t bytes_read;
	struct timeval tv;
	uint64_t eftd_ctr;
	int ret = 0;
	int index = 0;
	void *la9310_tbgen_master_counter = NULL;
	/* Validate CLI Args */
	validate_cli_args(argc, argv);
	/* Raise app priority to RT */
	/*sched_setscheduler(current, SCHED_FIFO, &param);*/
	sched_setscheduler(0, SCHED_FIFO, &param);

	/* Register Modem & TTI as per command line params supplied */
	tti_t.tti_eventfd = -1;
	ret = modem_tti_register(&tti_t, modem_id, tti_event_flag);
	if (ret < 0) {
		printf("%s failed...\n", __func__);
		goto error;
	}
	if (tti_event_flag == 1) {
		struct epoll_event ev, events[MAX_EVENTS];
		int epollfd, nfds;

		epollfd = epoll_create1(0);
		if (epollfd < 0) {
			printf("epoll create failed !\n");
			goto err;
		}

		ev.events = EPOLLIN;
		ev.data.fd = tti_t.tti_eventfd;
		if (epoll_ctl(epollfd, EPOLL_CTL_ADD,
				tti_t.tti_eventfd, &ev) == -1) {
			printf("Failed to create tti_eventfd\n");
			close(epollfd);
			goto err;
		}
		for (index = 0; index < (tti_count + 1); index++) {
			printf("Waiting for event\n");
			nfds = epoll_wait(epollfd, events,
					MAX_EVENTS, -1);
			if (nfds == 1) {
				gettimeofday(&tv, NULL);
				fflush(stdout);
				bytes_read = read(tti_t.tti_eventfd,
						&eftd_ctr,
						sizeof(uint64_t));
				if (bytes_read != sizeof(uint64_t)) {
					printf(
						"Error in reset counter : %ld\n"
						, bytes_read);
				}

			}
		}
		close(epollfd);
	} else {
		for (index = 0; index < (tti_count + 1); index++) {
			bytes_read = read(tti_t.dev_tti_handle, &eftd_ctr,
					sizeof(uint64_t));
			if (bytes_read != sizeof(uint64_t)) {
				printf("Read error. Exiting...\n");
				goto err;
			} else {
				/* Extract timestamp in usecs and populate the
				 *  data structure */
				gettimeofday(&tv, NULL);
				fflush(stdout);
			}
		}
	}
	/* Deregister TTI */
	modem_tti_deregister(&tti_t);
	exit(EXIT_SUCCESS);
	return 0;
err:
	modem_tti_deregister(&tti_t);
error:
	exit(EXIT_FAILURE);
}
