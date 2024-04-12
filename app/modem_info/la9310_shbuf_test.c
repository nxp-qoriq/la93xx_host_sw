// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "la9310_modinfo.h"

long long find_time_diff(struct timeval *diff,
		struct timeval *end_time,
		struct timeval *start_time
		)
{
	struct timeval temp_diff;

	if (diff == NULL)
		diff = &temp_diff;

	diff->tv_sec = end_time->tv_sec - start_time->tv_sec;
	diff->tv_usec = end_time->tv_usec - start_time->tv_usec;

	while (diff->tv_usec < 0) {
		diff->tv_usec += 1000000;
		diff->tv_sec -= 1;
	}

	return 1000000LL * diff->tv_sec + diff->tv_usec;

}

int main(int argc, char *argv[])
{
	int fd, seed, mod_id;
	void *addr;
	uint8_t *usr_buf;
	uint8_t *sh_buf;
	char dev_name[32];
	long size;
	off_t offset;
	struct timeval s_time, e_time, time_diff;
	long long latency = 0;
	modinfo_t mil = {0};
	modinfo_t *mi = &mil;
	int ret;

	if (argc < 2) {
		fprintf(stderr, "\nUsage:\t%s {modem-id} {address} {size}\n"
			"\tmodem-id : La9310 modem id\n"
			"\taddress <optional>  : Scratch/shared buffer address to access\n"
			"\tsize <optional>   : Size of buffer to access\n\n",
			argv[0]);
		return -1;
	}

	mod_id = strtol(argv[1], 0, 0);
	sprintf(dev_name, "/dev/%s%d", LA9310_DEV_NAME_PREFIX, mod_id);

	fd = open(dev_name, O_RDWR);
	if (-1 == fd) {
		perror("dev open failed:");
		return -1;
	}
	if (argc  >= 4) {
		offset = strtol(argv[2], 0, 0);
		size = strtol(argv[3], 0, 0);
	} else {
		ret = ioctl(fd, IOCTL_LA93XX_MODINFO_GET, (modinfo_t *) &mil);
		if (ret < 0) {
			printf("IOCTL_LA93XX_MODINFO_GET failed.\n");
			close(fd);
			return -1;
		}
		offset = mi->scratchbuf.host_phy_addr;
		size = mi->scratchbuf.size;
	}

	if (size <= 0) {
		perror("Invalid size\n");
		close(fd);
		return -1;
	}

	usr_buf = malloc(size);

	addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
	if (addr == MAP_FAILED) {
		perror("mmap failed:");
		close(fd);
		free(usr_buf);
		return -1;
	}

	sh_buf = addr;

	/* Accessing shared buffer */
	printf("shared buf address = %#lx len = %ld\n", offset, size);
	printf("Memset to %#lx len = %ld\n", offset, size);

	seed = 1;

	gettimeofday(&s_time, 0x0);

	for (int count = 0; count < 16; count++)
		memset(sh_buf, seed, size);

	gettimeofday(&e_time, 0x0);
	latency = find_time_diff(&time_diff, &e_time, &s_time);

	printf("Time taken to access shared buffer : %llu usec\n", latency);

	/* Accessing user buffer */
	printf("User buf address = %p len = %ld\n", usr_buf, size);
	printf("Memset to %p len = %ld\n", usr_buf, size);

	seed = 1;

	gettimeofday(&s_time, 0x0);

	for (int count = 0; count < 16; count++)
		memset(usr_buf, seed, size);

	gettimeofday(&e_time, 0x0);
	latency = find_time_diff(&time_diff, &e_time, &s_time);

	printf("Time taken to access user buffer : %llu usec\n", latency);
	munmap(addr, size);
	close(fd);

	return 0;
}
