// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023-2024 NXP
 */


#ifndef PHYSICAL_MEM_H_
#define PHYSICAL_MEM_H_
//! Structure which holds all necessary information from mmap() for DDR access and munmap()
typedef struct MEM_MAP_S {
    void *phy_addr;             //!< Physical addresss
    void *map_base;             //!< Virtual  base address, page aligned
    size_t map_size;            //!< Size of  allocated memory
    void *virt_addr;            //!< Virtual  addresss
} MEM_MAP_T;


extern int dev_mem_fd;;

void open_dev_mem();

void close_dev_mem();

void map_physical_region(MEM_MAP_T *map, unsigned long physical_addr, unsigned long size);

void unmap_physical_region(MEM_MAP_T *map);
#endif //PHYSICAL_MEM_H_
