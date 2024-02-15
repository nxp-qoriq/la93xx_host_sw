/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2020, 2024 NXP
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FILENAME_LEN	30
#define FATAL(str) do { \
			fprintf(stderr, str); \
			fprintf(stderr, "Error at line %d, file %s (%d)\n", \
			__LINE__, __FILE__, errno); exit(1); \
			} while (0)

void usage(void)
{
	printf("-i : input file, Input size bytes will be read from this file and copied to provided address\n");
	printf("-o : output file, Input size bytes will be read from provided address and copied to this file\n");
	printf("-a : address\n");
	printf("-s : size\n");
}

int main(int argc, char **argv)
{
	int fd;
	int opt;
	void *map_base, *virt_addr;
	unsigned int page_size, offset_in_page;
	off_t addr = 0;
	unsigned int page_count;
	char ifile[FILENAME_LEN] = {0};
	char ofile[FILENAME_LEN] = {0};
	unsigned int size = 0;
	FILE *iptr, *optr;
	size_t ret;

	while ((opt = getopt(argc, argv, "ha:i:o:s:")) != -1) {
		switch (opt) {
		case 'i':
			if (strlen(optarg) >= FILENAME_LEN) {
				printf("Input filename can be max 30 characters\n");
				exit(EXIT_FAILURE);
			}
			strcpy(ifile, optarg);
			break;
		case 'o':
			if (strlen(optarg) >= FILENAME_LEN) {
				printf("Output filename can be max 30 characters\n");
				exit(EXIT_FAILURE);
			}
			strcpy(ofile, optarg);
			break;
		case 'a':
			addr = strtoull(optarg, NULL, 16);
			break;
		case 's':
			size = strtoul(optarg, NULL, 10);
			break;
		case 'h':
		default:
			usage();
			return 0;
		}
	}

	if (((ifile[0] == 0) && (ofile[0] == 0)) ||
	    (addr == 0) ||
	    (size == 0))
		FATAL("Invalid parameters\n");

	if (ifile[0])
		printf("input file: %s\n", ifile);
	if (ofile[0])
		printf("outfile file: %s\n", ofile);
	printf("addr: 0x%lx, size: %u\n", addr, size);

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd == -1)
		FATAL("Error opening /dev/mem\n");
	printf("/dev/mem opened.\n");
	fflush(stdout);

	page_size = getpagesize();
	offset_in_page = addr & (page_size - 1);
	page_count = size / page_size;
	if (!page_count)
		page_count = 1;
	if ((page_count * page_size) < size)
		page_count += 1;

	map_base = mmap(NULL,
			page_count * page_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd,
			addr & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED)
		FATAL("mmap failed\n");

	virt_addr = (char *)map_base + offset_in_page;

	if (ifile[0]) {
		iptr = fopen(ifile, "rb");
		if (!iptr)
			FATAL("Input file open failed\n");
		ret = fread(virt_addr, size, 1, iptr);
		printf("in file copy: %lu\n", ret);
		if (fclose(iptr))
			FATAL("Input file close failed\n");
	}


	if (ofile[0]) {
		optr = fopen(ofile, "wb");
		if (!optr)
			FATAL("Output file open failed\n");
		ret = fwrite(virt_addr, size, 1, optr);
		printf("out file copy: %lu\n", ret);
		if (fclose(optr))
			FATAL("Output file close failed\n");
	}


	if (munmap(map_base, page_count * page_size) == -1) {
		close(fd);
		FATAL("Memory unmap failed\n");
	}

	close(fd);

	return 0;
}
