// SPDX-License-Identifier: BSD-3-Clause
//Copyright 2024 NXP


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>
#include <diora_ecspi_api.h>

static void ecspi_read_write_test(void *ecspi_base, uint32_t chan)
{
	uint32_t addr_val, cnt;
	uint16_t rx_val, tx_val;
	error_t ret;
	int flag = 1;
	volatile  uint64_t prev, curr, delta;

	pr_info("\r\n====Test Receive=====\r\n");
	for (cnt = 1; cnt <= 10; cnt++) {
		addr_val  = 0x00A7; //read as 0x6565
		ret = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
		if (ret == 0)
			pr_info("RegAddr 0x%08X RcvdVal 0x%X\r\n", addr_val, rx_val);
		addr_val  = 0x0086; //read as 0x4905
		ret =  diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
		if (ret == 0)
			pr_info("RegAddr 0x%08X RcvdVal 0x%X\r\n", addr_val, rx_val);
	}

	pr_info("\r\n====Test TX =====\r\n");
	tx_val = 0xAAAA;
	for (cnt = 1; cnt <= 10; cnt++) {
		addr_val = 0x002A;
		ret = diora_phal_write16(ecspi_base, chan, addr_val, tx_val);
		ret  = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
		if ((rx_val & 0xffff) == (tx_val & 0xffff))
			pr_info("Success:---> Tx 0x%X Rx 0x%X ==\r\n", tx_val, rx_val);
		else
			pr_info("Fail Tx:---> 0x%X Rx 0x%X ==\r\n", tx_val, rx_val);

		if (flag) {
			tx_val = 0x5555;
			flag = 0;
		} else {
			tx_val = 0xAAAA;
			flag = 1;
		}
	}

	addr_val  = 0x00A7; //read as 0x6565
	/*Get pmu counter cpu cycles before test start */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(prev));
	for (cnt = 1; cnt <= 100; cnt++)
		ret = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr - prev;
	if (delta)
		printf("Read cpu cycles (hex 0x%lx dec %lu) time taken %ld(nano second)  \r\n", delta, delta, ((delta * 10 )/16) / 100);
	else
		printf("Read No diff , something wrong\r\n");

	addr_val = 0x002A;
	tx_val = 0x5555;
	/*Get pmu counter cpu cycles before test start */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(prev));
	for (cnt = 1; cnt <= 100; cnt++)
		ret = diora_phal_write16(ecspi_base, chan, addr_val, tx_val);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr-prev;
	if (delta)
		printf("Write cpu cycles (hex 0x%lx dec %lu) time taken %ld(nano second)  \r\n", delta, delta, ((delta * 10 )/16) / 100);
	else
		printf("Write No diff , something wrong\r\n");

}


int main(int argc, char *argv[])
{
	void *ecspi_base;

	ecspi_base = diora_phal_rw_init(IMX8MP_ECSPI_1);

	if (ecspi_base != NULL)
		ecspi_read_write_test(ecspi_base, IMX8MP_ECSPI_1);
	else
		return -1;

	diora_phal_rw_deinit(IMX8MP_ECSPI_1);

	exit(EXIT_SUCCESS);
}
