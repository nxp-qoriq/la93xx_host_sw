/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2017, 2021-2022 NXP
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	uint32_t size, val;
	uint32_t *p, *p_start;
	int fd, i;

	if (argc < 4) {
		fprintf(stderr, "usage: %s <file> <size> <0xval>\n", argv[0]);
		return 1;
	}

	fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}

	sscanf(argv[2], "%d", &size);
	sscanf(argv[3], "%x", &val);

	if (0x10000 < size) {
		printf("size should be less than 0x10000\n");
		return 1;
	}

	p = malloc(size);

	if (!p) {
		printf("Malloca filed\n");
		return 1;
	}

	p_start = p;
	printf("malloc ptr %p\n", p);
	for (i = 0; i < size/4; i++) {
		*p = val;
		p++;
	}
	write(fd, p_start, size);
	printf("Val 0x%x, size %d\n", val, size);

	if (close(fd) == -1) {
		perror("close");
		return 1;
	}

	free(p_start);
	return 0;
}
