// SPDX-License-Identifier: BSD-3-Clause
//Copyright 2024 NXP

#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <string.h>
#include <sched.h>
#include <signal.h>
#include <diora_ecspi_api.h>

#define DIORA_TEST 1
#define MAX_SPI_FRAME  100
static uint32_t ecspi_ch_id;
static int cpu_to_affine = -1;

void print_usage_message(void)
{
	printf("Usage: ./ecspi_test_app ecspi_ch_id(1..3)  -c(cpu number to affine(1..4))\r\n");
	fflush(stdout);
	exit(EXIT_SUCCESS);
}


void validate_cli_args(int argc, char *argv[])
{
	int opt;

	if (argc <= 1)
		print_usage_message();

	ecspi_ch_id  = atoi(argv[1]);
	if (!(ecspi_ch_id >= 1 && ecspi_ch_id <= 3))
		printf("Invalid ECSPI Channel number, Valid values are between 1..3\r\n");

	while ((opt = getopt(argc, argv, ":c:h")) != IMX_ECSPI_FAIL) {
		switch (opt) {
		case 'c':
			cpu_to_affine = atoi(optarg);
			if (!(cpu_to_affine >= 1 && cpu_to_affine <= 4)) {
				printf("Valid values for cpu to affine is between 1..4\r\n");
				print_usage_message();
			}
			break;
		case 'h':
			print_usage_message();
			break;
		case '?':
		/*  FALLTHROUGH */
		default:
			print_usage_message();
			break;
		}
	}
}

static void ecspi_read_write_test(void *ecspi_base, uint32_t chan)
{
	uint32_t addr_val, cnt;
	uint16_t rx_val, tx_val, expected_read;
	error_t ret;
	int flag = 1;
	volatile  uint64_t prev, curr, delta;

	printf("\r\n====Test Receive=====\r\n");
	for (cnt = 1; cnt <= MAX_SPI_FRAME; cnt++) {
		if (flag) {
			addr_val  = 0x00A7;
			expected_read = 0x6565;
		} else {
			addr_val  = 0x0086;
			expected_read = 0x4905;
		}
		ret = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
		if ((ret == 0) && (rx_val == expected_read))
			printf("Success--> RegAddr 0x%08X RcvdVal 0x%X Expected 0x%X \r\n", addr_val, rx_val, expected_read);
		else
			printf("Fail--> RegAddr 0x%08X RcvdVal 0x%X Expected 0x%X \r\n", addr_val, rx_val, expected_read);
	}

	printf("\r\n====Test TX =====\r\n");
	tx_val = 0xAAAA;
	for (cnt = 1; cnt <= MAX_SPI_FRAME; cnt++) {
		addr_val = 0x002A;
		ret = diora_phal_write16(ecspi_base, chan, addr_val, tx_val);
		ret  = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
		if ((rx_val & 0xffff) == (tx_val & 0xffff))
			printf("Success:---> Tx 0x%X Rx 0x%X\r\n", tx_val, rx_val);
		else
			printf("Fail Tx:---> 0x%X Rx 0x%X\r\n", tx_val, rx_val);

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
	for (cnt = 1; cnt <= MAX_SPI_FRAME; cnt++)
		ret = diora_phal_read16(ecspi_base, chan, addr_val, &rx_val);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr - prev;
	if (delta)
		printf("Read cpu cycles (hex 0x%lx dec %lu) time taken %ld (nano second, for 32 bit frame)\r\n", delta, delta, ((delta * 10)/16) / MAX_SPI_FRAME);
	else
		printf("Read No diff , something wrong\r\n");

	addr_val = 0x002A;
	tx_val = 0x5555;
	/*Get pmu counter cpu cycles before test start */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(prev));
	for (cnt = 1; cnt <= MAX_SPI_FRAME; cnt++)
		ret = diora_phal_write16(ecspi_base, chan, addr_val, tx_val);
	/*Get pmu counter cpu cycles after test  */
	asm volatile("isb;mrs %0, pmccntr_el0" : "=r"(curr));

	/*Calculate delta and print*/
	delta = curr-prev;
	if (delta)
		printf("Write cpu cycles (hex 0x%lx dec %lu) time taken %ld (nano second for 32 bit frame)\r\n", delta, delta, ((delta * 10)/16) / MAX_SPI_FRAME);
	else
		printf("Write No diff , something wrong\r\n");

}

void sigint_handler(int signum)
{
	int ecspi_chan = 0;
	(void)signum;
	printf("===> CTRL ^ C, Invoke cleanup handler....\r\n");
	signal(SIGINT, SIG_DFL);

	for (ecspi_chan = 0;  ecspi_chan < IMX8MP_ECSPI_MAX_DEVICES; ecspi_chan++)
		diora_phal_rw_deinit(ecspi_chan);
	exit(0);
}


int main(int argc, char *argv[])
{
	void *ecspi_base;
	uint32_t ecspi_chan;

	validate_cli_args(argc, argv);
	ecspi_chan = ecspi_ch_id - 1;
	if (cpu_to_affine != IMX_ECSPI_FAIL)  {
		printf("CPU to affine %d \r\n", cpu_to_affine);
		cpu_set_t set;

		CPU_ZERO(&set);
		CPU_SET(cpu_to_affine - 1, &set);
		if (sched_setaffinity(0, sizeof(cpu_set_t), &set) == IMX_ECSPI_FAIL) {
			printf("sched_setaffinity failed..\r\n");
			return IMX_ECSPI_FAIL;
		}
	}
#ifdef DIORA_TEST
	ecspi_base = diora_phal_rw_init(ecspi_chan);
#else
	ecspi_clk_t clk;

	clk.ecspi_root_clk = IMX8MP_ECSPI_CLK_SYSTEM_PLL1_CLK;
	clk.ecspi_ccm_target_root_pre_podf_div_clk = 0x0; /* Pre divider, Target Register (CCM_TARGET_ROOTn) bit 16 to 18 */
	clk.ecspi_ccm_target_root_post_podf_div_clk = 0x0; /* Post divider, Target Register (CCM_TARGET_ROOTn) bit 0 to 5  */
	clk.ecspi_ctrl_pre_div_clk = 0x5; /* 0x5 Pre divider, Control Register (ECSPIx_CONREG) bit 12 to 15. */
	clk.ecspi_ctrl_post_div_clk = 0x3; /* 0x3 Post divider Control Register (ECSPIx_CONREG) bit 8 to 11. */
	ecspi_base = imx_spi_init_with_clk(ecspi_chan, clk);
	memset(&clk, 0x00, sizeof(clk));
	imx_get_spi_clk_config(ecspi_base, ecspi_chan, &clk);

	switch (clk.ecspi_root_clk) {
	case 0:
		printf("24M_REF_CLK Selected\r\n");
		break;
	case 4:
		printf("SYSTEM_PLL1_CLK Selected\r\n");
		break;
	case 3:
		printf("SYSTEM_PLL1_DIV5 Selected\r\n");
		break;
	case 2:
		printf("SYSTEM_PLL1_DIV20 Selected\r\n");
		break;
	case 6:
		printf("SYSTEM_PLL2_DIV4 Selected\r\n");
		break;
	case 1:
		printf("SYSTEM_PLL2_DIV5 Selected\r\n");
		break;
	case 5:
		printf("SYSTEM_PLL3_CLK Selected\r\n");
		break;
	case 7:
		printf("AUDIO_PLL2_CLK Selected\r\n");
		break;
	default:
		pr_debug("Unsupported root clock %d\r\n", clk.ecspi_root_clk);
		break;
}
printf("ecspi_ccm_target_root_pre_podf_div_clk = 0x%X \r\n", clk.ecspi_ccm_target_root_pre_podf_div_clk);
printf("ecspi_ccm_target_root_post_podf_div_clk = 0x%X \r\n", clk.ecspi_ccm_target_root_post_podf_div_clk);
printf("ecspi_ctrl_pre_div_clk = 0x%X \r\n", clk.ecspi_ctrl_pre_div_clk);
printf("ecspi_ctrl_post_div_clk = 0x%X \r\n", clk.ecspi_ctrl_post_div_clk);

#endif
	if (ecspi_base != NULL)
		ecspi_read_write_test(ecspi_base, ecspi_chan);
	else
		return IMX_ECSPI_FAIL;

	diora_phal_rw_deinit(ecspi_chan);

	exit(EXIT_SUCCESS);
}
