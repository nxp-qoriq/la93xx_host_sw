// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023-2024 NXP
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include "physical_mem.h"
#include "log.h"

#define CMDLINE_OPTIONS "f:a:c:r:d:"

#define __APP_VER__ "1.0.1"

typedef struct RUN_OPT_S
{
  char 		filename[255];
  uint64_t 	phyAddr;
  int 		chunkSize;
  int 		revSize;
  int 		rev;
  int 		delay;

} RUN_OPT_T;
RUN_OPT_T runOpt;


void soft_memcpy(char *dst, const char *src, size_t len, int chunk_size,  int chunk_delay_us) {
  int idx;
  int rem;
  int parts;

  if (chunk_size == 0) {
    chunk_size = len;
  }

  rem = len % chunk_size;
  parts = len / chunk_size;

  for (idx = 0; idx < parts; idx++){
    memcpy(dst, src, chunk_size);
    dst += chunk_size;
    src += chunk_size;
    if (chunk_delay_us > 0) {
      usleep(chunk_delay_us);     
    }
  }
  if (rem) {
    memcpy(dst, src, chunk_size);
  }

}


void copy_from_file(char *filename, uint64_t physicalAddr, int chunkSize, int delay_us) {
	FILE * pFile;
	long lSize;
	char * buffer;
	size_t result;
	MEM_MAP_T tempMap;

	pFile = fopen(filename,"rb");
	if (!pFile)
	{
		printf("Unable to open file!");
		exit(-1);
	}


	fseek(pFile, 0L, SEEK_END);
	lSize = ftell(pFile);
	rewind(pFile);

	buffer = (char*) malloc (lSize + 64);
	if (buffer == NULL) {fputs ("Cannot allocate enough memory for copy", stderr); exit (2);}
	result = fread (buffer,1,lSize,pFile);
	fclose (pFile);
	if (result != lSize) {fputs ("Reading error",stderr); exit (3);}
    
	map_physical_region(&tempMap, physicalAddr, lSize);

	if (!tempMap.virt_addr) {
		fputs ("Cannot mmap physical address / size", stderr); 
		exit (2);
	}


	soft_memcpy(tempMap.virt_addr, buffer, lSize, chunkSize, delay_us);


	free (buffer);
	unmap_physical_region(&tempMap);

}


void copy_to_file(char *filename, uint64_t physicalAddr, int chunkSize, int size, int delay_us) {
	FILE * pFile;
	char * buffer;
	MEM_MAP_T tempMap;

	pFile = fopen(filename,"wb");
	if (!pFile)
	{
		printf("Unable to open file!");
		exit(-1);
	}



	buffer = (char*) malloc (size + 64);
	if (buffer == NULL) {fputs ("Cannot allocate enough memory for copy", stderr); exit (2);}

    
	map_physical_region(&tempMap, physicalAddr, size);

	if (!tempMap.virt_addr) {
		fputs ("Cannot mmap physical address / size", stderr); 
		exit (2);
	}

	soft_memcpy(buffer, tempMap.virt_addr, size, chunkSize, delay_us);

	fwrite(buffer,1,size,pFile);
	fclose (pFile);


	free (buffer);
	unmap_physical_region(&tempMap);

}


void print_help()
{
  printf("Build date: %s \n", 		__DATE__);
  printf("Build time: %s \n", 		__TIME__);
  printf("Toolchain version: %s \n", 	__VERSION__);
  printf("App version : %s \n", 	__APP_VER__);

  printf("Usage guide:\n");
  printf("Options: -" CMDLINE_OPTIONS "\n");
  printf("\t -f binary file\n");
  printf("\t -a physical address\n");
  printf("\t -c chunk size -- transaction size\n");
  printf("\t -d u delay between transactions\n");
  printf("\t -r reverse mem->file followed by size to dump\n");

  printf("Examples:\n");
  printf("\t./bin2mem -f data.bin -a 0xC10303040\n");

}


void parse_arguments(int argc, char* argv[], RUN_OPT_T* runOpt)
{
  int argNum = 0;
  int opt;

  runOpt->chunkSize = 0;
  runOpt->rev 		= 0;
  runOpt->delay 	= 0;
  while ((opt = getopt(argc, argv, CMDLINE_OPTIONS)) != -1) {
    switch (opt) {
      case 'f':
	      strcpy(runOpt->filename, optarg);
	      argNum++;
      break;
      case 'a':
	      runOpt->phyAddr = strtoul(optarg,NULL,0);
	      argNum++;
      break;
      case 'c':
		runOpt->chunkSize = atoi(optarg);
		break;

      case 'r':
        runOpt->rev = 1;
		runOpt->revSize = atoi(optarg);
		break;
	  case 'd':
	  	runOpt->delay = atoi(optarg);
	  	break;
      default: /* '?' */
      print_help();
      exit(EXIT_FAILURE);
    }
  }

  if (argNum != 2) {
    print_help();
    exit(-1);
  }

}


int main(int argc, char* argv[])
{
  log_init();

  // Interpret command line arguments
  parse_arguments(argc, argv, &runOpt);
  open_dev_mem();
  if (runOpt.rev) {
  	copy_to_file(runOpt.filename, runOpt.phyAddr, runOpt.chunkSize, runOpt.revSize, runOpt.delay);
  } else {
  	copy_from_file(runOpt.filename, runOpt.phyAddr, runOpt.chunkSize, runOpt.delay);  	
  }

  return 0;
} 

