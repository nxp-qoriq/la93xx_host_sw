// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023-2024 NXP
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <ctype.h>
#include "physical_mem.h"
#include "log.h"

#define PAGE_SIZE 4096UL
#define PAGE_MASK (PAGE_SIZE - 1)
#define PRINT_MAP_INFO 0

int dev_mem_fd;
static int dev_mem_ref_ctr;
void open_dev_mem()
{
    if (dev_mem_ref_ctr == 0) {
        /* Open /dev/mem driver - requires root permission */
        if ((dev_mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
            printf("Error: /dev/mem could not be opened.\n");
            perror("open");
            exit(1);
        }
    }
    dev_mem_ref_ctr++;
}

// TBD: need to add this to nice exit
void close_dev_mem()
{
    dev_mem_ref_ctr--;
    if (dev_mem_ref_ctr == 0){
        close(dev_mem_fd);
    }
}


void map_physical_region(MEM_MAP_T *map, unsigned long physical_addr, unsigned long size)
{
    unsigned long end_addr;
    unsigned long start_page, end_page;

    open_dev_mem();
    /* Calculate page boundaries */
    end_addr = physical_addr + size - 1;
    start_page = physical_addr & ~PAGE_MASK;
    end_page = end_addr & ~PAGE_MASK;
    map->map_size = end_page - start_page + PAGE_SIZE;

    /* Map page(s) */
#if PRINT_MAP_INFO
    DBG_PRINT(INFO_LEVEL, "mmap :phy : 0x%lx, size : 0x%lx; start_page : 0x%lx, size : 0x%lx\n",
         physical_addr, size, start_page, map->map_size);
#endif
    map->map_base = mmap(0, map->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_mem_fd, start_page);
    if (map->map_base == (void *) -1) {
        printf("Error: Memory map failed.\n");
        perror("mmap");
        close(dev_mem_fd);
        exit(1);
    } else {
        map->phy_addr  =(void*) physical_addr;
        map->virt_addr = map->map_base + (physical_addr & PAGE_MASK);
    }
#if PRINT_MAP_INFO
     DBG_PRINT(INFO_LEVEL, "MMAP : 0x%lx <---> 0x%lx |||| 0x%lx <---> 0x%lx \n",
         map->phy_addr, map->virt_addr,
         start_page, map->map_base);
#endif   
}

// TBD: need to add this to nice exit
void unmap_physical_region(MEM_MAP_T *map)
{
    if (munmap(map->map_base, map->map_size) == -1) {
        printf("Error: Memory unmap failed.\n");
        exit(1);
    }
    close_dev_mem();
}
