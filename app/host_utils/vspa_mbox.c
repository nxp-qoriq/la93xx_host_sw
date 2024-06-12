/* Copyright 2022-2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
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

#define dsb(opt) asm volatile("dsb " #opt : : : "memory")
#define dmb(opt) asm volatile("dmb " #opt : : : "memory")

static inline uint32_t
ioread32(const volatile void *addr)
{
    uint32_t val;

    asm volatile(
            "ldr %w[val], [%x[addr]]"
            : [val] "=r" (val)
            : [addr] "r" (addr));

    dsb(ld);
    return val;
}

static inline uint64_t
ioread64(const volatile void *addr)
{
    uint64_t val;

    asm volatile(
            "ldr %x[val], [%x[addr]]"
            : [val] "=r" (val)
            : [addr] "r" (addr));

    dsb(ld);
    return val;
}

static inline void
iowrite32(uint32_t val, volatile void *addr)
{
    dsb(st);
    asm volatile(
            "str %w[val], [%x[addr]]"
            :
            : [val] "r" (val), [addr] "r" (addr));
}

static inline void
iowrite64(uint64_t val, volatile void *addr)
{
    dsb(st);
    asm volatile(
            "str %x[val], [%x[addr]]"
            :
            : [val] "r" (val), [addr] "r" (addr));
}

static inline uint64_t read_from_cfg(char *filename)
{
    uint64_t val = 0;
    char cfg[256], *stopstring;
    FILE *fp;

    if((fp = fopen(filename, "r")) == NULL) {
        return 0;
    } else {
        if (fgets(cfg, 256, fp) != NULL) {
            val = strtoull(cfg, &stopstring, 16);
			fclose(fp);
        }
    }

    return val;
}


static inline uint64_t get_modem_ccsr_base()
{
    FILE *fp;
    char path[1035], *stopstring;
    uint64_t val = 0;

    val = read_from_cfg("/usr/local/etc/bar0.cfg");
    if(val == 0) {
        //printf("configuration file not found, try the default device.\n");
        /* Open the command for reading. */
        //fp = popen("lspci -v -s 0002:01:00.0 -d :1c30 | grep Memory | head -n 1 | awk '{print $3; }'", "r");
    	fp = popen("dmesg |grep 'BAR:0'|cut -f 6 -d ':'|cut -f 1 -d ' '", "r");
        if (fp == NULL) {
            printf("Failed to run command\n" );
            exit(1);
        }

        /* Read the output a line at a time - output it. */
        while (fgets(path, sizeof(path), fp) != NULL) {
            val = strtoull(path, &stopstring, 16);
            printf("\n%lx\n", val);
        }
        /* close */
        pclose(fp);
    }

    if(val == 0) {
        printf("default device not match, use the default bar 0 address.\n");
        val = 0x18000000;
    }
    printf("%s: Base PCI addr phy 0x%lx \n", __func__, val);
    return val;
}

#define MAILBOX_ADDR(base, mbox, direction, core_idx) \
    (base + 0x680 + direction * 0x10 + mbox * 8 + 0x4000 * core_idx)
/* Assumption: msg is 4 32bit words. */
static inline void
vspa_mbox_send(uint64_t vspa_addr, int core_idx, int mbox_id, uint32_t msb, uint32_t lsb)
{
    uint64_t addr = MAILBOX_ADDR(vspa_addr, mbox_id, 0, core_idx);

    //printf("%s: VSPA[%d] addr 0x%lX  val 0x%lX.\n",__func__, core_idx, addr, (uint64_t)(msb)<<32|(uint64_t)lsb);
    iowrite32(msb, (void*)addr);
    iowrite32(lsb, (void*)(addr+4));
}

static int
vspa_mbox_recv(uint64_t vspa_addr, int core_idx, int mbox_id)
{
    uint64_t addr = vspa_addr + 0x660 + 0x4000 * core_idx;
    uint64_t timeout = 1000000;
    uint32_t msb, lsb;

    while (timeout && !(ioread32((void*)addr) & (1<<(mbox_id /*+ 2*/))))
        timeout--;

    if (!timeout) {
        printf("%s: VCPU:%d MBox:%d is not responding!!\n",__func__, core_idx, mbox_id);
        return -1;
    }

    addr = MAILBOX_ADDR(vspa_addr, mbox_id, 1, core_idx);
    msb = ioread32((void*)addr);
    lsb = ioread32((void*)(addr+4));
    printf("Received from VSPA:%d, MBox:%d, MSB:0x%08x, LSB:0x%08x.\n", core_idx, mbox_id, msb, lsb);
    return 0;

}

static inline void
vspa_mbox_clear(uint64_t vspa_addr, int core_idx, int mbox_id)
{
    //uint64_t addr0 = MAILBOX_ADDR(vspa_addr, mbox_id, 1, core_idx);
    //ioread64((void*)addr0);

    if (mbox_id == 0)
        iowrite32(1 << 14, (void*)(vspa_addr + 0x10 + 0x4000 * core_idx));
    else
        iowrite32(1 << 15, (void*)(vspa_addr + 0x10 + 0x4000 * core_idx));
}

static void usage(char *argv[])
{
    printf("%s send core_id mbox_id msb32 lsb32\n", argv[0]);
    printf("%s recv core_id mbox_id\n", argv[0]);
}

int main (int argc, char *argv[])
{
    int fd, size;
    uint64_t modem_ccsr_base, vspa_addr_phys, vspa_addr;
    uint32_t core_id=0, mbox_id=0, msb32=0, lsb32=0;

    if(argc < 4) {
        printf("Wrong number of parameters\n");
        usage(argv);
        return -1;
    }
    
    if (!((strcmp(argv[1], "send") == 0) || (strcmp(argv[1], "recv") == 0))) {
        printf("command must be either 'send' or 'recv'\n");
        usage(argv);
        return -1;
    }

    core_id = strtoul(argv[2], NULL, 10);
    if (core_id >= 8) {
        printf("Invalid vspa core id, should be within [0,7].\n");
        return -1;
    }

    mbox_id = strtoul(argv[3], NULL, 10);
    if (mbox_id > 1) {
        printf("Invalid mail box id, should be either 0 or 1.\n");
        return -1;
    }

    fd = open("/dev/mem", O_RDWR);
    size = 0x20000;

    //vspa_addr_phys = 0x18000000 + 0x1000000;
    vspa_addr_phys = get_modem_ccsr_base() + 0x1000000;
    vspa_addr = (uint64_t)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, vspa_addr_phys);

    if (MAP_FAILED == (void *)vspa_addr) {
        printf("Failed mmap mbox memory to user-space\n");
        close(fd);
        return -1;
    }
    //printf("%s: mailbox_addr_phys 0x%lX vir_addr 0x%lX\n",__func__, vspa_addr_phys, vspa_addr);

    if ((strcmp(argv[1], "send") == 0)) {
        if (argc != 6) {
            printf("Wrong number of parameters\n");
            usage(argv);
            return -1;
        }

        msb32 = strtoul(argv[4], NULL, 16);
        lsb32 = strtoul(argv[5], NULL, 16);

        printf("Ready to send 0x%08x-0x%08x to vspa:%d mail box:%d\n", msb32, lsb32, core_id, mbox_id);
        vspa_mbox_send(vspa_addr, core_id, mbox_id, msb32, lsb32);

    } else if ((strcmp(argv[1], "recv") == 0)) {
        if (argc != 4) {
            printf("wrong number of parameters\n");
            usage(argv);
            return -1;
        }

        vspa_mbox_recv(vspa_addr, core_id, mbox_id);
        vspa_mbox_clear(vspa_addr, core_id, mbox_id);
    }

  //  close(fd);

    return 0;
}
