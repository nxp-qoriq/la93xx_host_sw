// SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
/*
 * Copyright 2017, 2021-2022 NXP
 */

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <la9310_base.h>
#include "la9310_test.h"
#include "la9310_v2h_if.h"

#define TIME_SEC_TO_NS      1000000000ULL

char la9310_device_name[LA9310_DEVICE_NAME_MAX_LEN];

int packet_dump = 1;			/* Default: Dump the packet */

static void  __exit la9310_test_exit(void)
{
#ifdef NLM_ENABLE_V2H
	int err = 0;
#endif
	struct la9310_dev *la9310_dev =
				   get_la9310_dev_byname(la9310_device_name);

	if (la9310_dev == NULL) {
		pr_err("No LA9310 device name found during %s\n", __func__);
		return;
	}
#ifdef NLM_ENABLE_V2H
	err = v2h_callback_test_deinit();
	if (err < 0)
		dev_err(la9310_dev->dev, "Failed to unregister V2H Callback\n");
#endif

}

static int __init la9310_test_init(void)
{
	int err = 0;
	struct la9310_dev *la9310_dev;

	if (la9310_device_name[0] == '\0') {
		pr_err("Provide device name through module param\n");
		pr_info("Ex. modprobe la9310demo.ko device=nlm0\n");
		return -ENODEV;
	}

	la9310_dev = get_la9310_dev_byname(la9310_device_name);
	if (la9310_dev == NULL) {
		pr_err("No LA9310 device named %s\n", la9310_device_name);
		pr_err("Provide valid la9310 device name(nlmX)\n");
		return -ENODEV;
	}

#ifdef NLM_ENABLE_V2H
	err = v2h_callback_test_init(la9310_dev);
	if (err < 0)
		dev_err(la9310_dev->dev, "Failed to register V2H Callback\n");
#endif

	return err;
}

module_param(packet_dump, int, 0);
module_param_string(device, la9310_device_name, sizeof(la9310_device_name), 0);
MODULE_PARM_DESC(device, "LA9310 Device name(wlan_monX)");
module_init(la9310_test_init);
module_exit(la9310_test_exit);
MODULE_LICENSE("GPL");
