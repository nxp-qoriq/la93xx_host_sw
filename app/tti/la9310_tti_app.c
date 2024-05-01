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
#include <sys/syscall.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct tti_stats {
	        suseconds_t micro_sec;
} tti_stats_t;

static int modem_id;
static int tti_count;
static int tti_event_flag;
static int cpu_id = -1;
cpu_set_t  mask;

#ifndef MAX_MODEM
#define MAX_MODEM 1
#endif
/* Every modem can have one to four TTI IDs */
#define MAX_COUNT		1000000
#define MAX_CPU			8
#define	LA9310_RFIC_SCHED_PRIORITY	99
#define MAX_EVENTS 1
#define TTI_INTERRUPT_INTERVAL_USEC 500

void print_usage_message(void)
{
	printf("Usage : ./app_name modem_id tti_count_max -c cpu_id [options] \n");
	printf("\n");
	printf("options:\n");
	printf("\n-e : eventMode\n");
	printf("\n");
	printf("mandatory:\n");
	printf("\tmodem_id         -- decimal |	0..%d\n", MAX_MODEM-1);
	printf("\ttti_count_max	 -- decimal |	0..%d\n", MAX_COUNT-1);
	printf("\tcpu_id	 -- decimal |	0..%d\n", MAX_COUNT-1);
	fflush(stdout);

	exit(EXIT_SUCCESS);
}

void validate_cli_args(int argc, char *argv[])
{
	int opt;
	if (argc <=1)
		print_usage_message();

	if ((strcmp(argv[1], "-h") == 0)  ||
			(strcmp(argv[1], "-H") == 0))
		print_usage_message();

	if (argc < 3) {
		print_usage_message();
		return;
	}

	modem_id = atoi(argv[1]);
	tti_count = atoi(argv[2]);
	tti_event_flag = 0;

	while((opt = getopt(argc, argv, ":c:eh")) != -1) {
		switch (opt) {
		case 'c':
			cpu_id = atoi(optarg);
			break;
		case 'e':
			tti_event_flag = 1;
			break;
		default:
			print_usage_message();
			break;
		}
	}
}

int rte_sys_gettid(void)
{
	return (int)syscall(SYS_gettid);
}

static inline void assign_to_core(int core_id)
{
	CPU_ZERO(&mask);
	CPU_SET(core_id, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}

struct sched_param param = { .sched_priority = LA9310_RFIC_SCHED_PRIORITY };
int main(int argc, char *argv[])
{
	struct tti tti_t;
	ssize_t bytes_read;
	struct timeval tvCurr;
	uint64_t pv_sec = 0, pv_usec = 0;
	uint64_t eftd_ctr;
	int ret = 0;
	int index = 0;
	int diff, cnt_up=0, cnt_down = 0;
	int min_t = 10000, max_t = -1, tid;
	tti_stats_t *stats_arr;
	char command[32];
	/* Validate CLI Args */
	validate_cli_args(argc, argv);

	if (cpu_id > 0)
		assign_to_core(cpu_id);

	/* Raise app priority to RT */
	/*sched_setscheduler(current, SCHED_FIFO, &param);*/
	sched_setscheduler(0, SCHED_FIFO, &param);

	tid = rte_sys_gettid();
	sprintf(command, "chrt -p 99 %d", tid);
	ret = system(command);

	stats_arr = (tti_stats_t *)malloc((tti_count + 1) *
			sizeof(tti_stats_t));
	if (!stats_arr) {
		printf("OOM while allocating stats array...\n");
		goto error;
	}
	memset(stats_arr, 0, (tti_count + 1) * sizeof(tti_stats_t));

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
			nfds = epoll_wait(epollfd, events,
					MAX_EVENTS, -1);
			if (nfds == 1) {
				gettimeofday(&tvCurr, NULL);
				if (index == 0)
					stats_arr[index].micro_sec = tvCurr.tv_usec;
				else
					stats_arr[index].micro_sec = ((tvCurr.tv_sec - pv_sec)*1000000L + tvCurr.tv_usec) - pv_usec;
				pv_sec = tvCurr.tv_sec;
				pv_usec = tvCurr.tv_usec;
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
				gettimeofday(&tvCurr, NULL);
				if (index == 0)
					stats_arr[index].micro_sec = tvCurr.tv_usec;
				else
					stats_arr[index].micro_sec = ((tvCurr.tv_sec - pv_sec)*1000000L + tvCurr.tv_usec) - pv_usec;
				pv_sec = tvCurr.tv_sec;
				pv_usec = tvCurr.tv_usec;
				fflush(stdout);
			}
		}
	}

	for (index = 0; index < tti_count; index++) {
		diff = stats_arr[index].micro_sec;
		if (index <= 5)
			continue;
		if (diff > max_t)
			max_t = diff;
		else if (diff < min_t)
			min_t = diff;
		if (diff > (TTI_INTERRUPT_INTERVAL_USEC + 10))
			cnt_up++;
		else if (diff < (TTI_INTERRUPT_INTERVAL_USEC - 10))
			cnt_down++;

		printf("tti_count=%d\t" "interrupt freq: %d\n", index, diff);
	}
	printf("Max time=%d\t Min time: %d\t Up count:%d\t Down count:%d\n",
			max_t, min_t, cnt_up, cnt_down);

	/* Deregister TTI */
	modem_tti_deregister(&tti_t);
	exit(EXIT_SUCCESS);
	return 0;
err:
	modem_tti_deregister(&tti_t);
error:
	exit(EXIT_FAILURE);
}
