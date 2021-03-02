/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2017-2018, 2021-2022 NXP
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include "la9310_v2h_callback.h"
#include "la9310_v2h_if.h"
#include "la9310_base.h"
#include "la9310_test.h"
#include "la9310_v2h_callback_test.h"

void *cookie;
#ifdef DEBUG_V2H_TEST
static int pkt_ctr;
#endif

void callback_func(struct sk_buff *skb_ptr, void *cookie)
{
#ifdef DEBUG_V2H_TEST
	uint8_t *ptr;
	int i;
#endif
	struct la9310_dev *la9310_dev = (struct la9310_dev *)cookie;

	if (NULL != skb_ptr) {
		#ifdef DEBUG_V2H_TEST
		pkt_ctr++;
		dev_info(la9310_dev->dev, "V2H  pkt len 0x%x\n", skb_ptr->len);
		ptr = (uint8_t *)skb_ptr->data;
		for (i = 0; i < 4 ; i++)  {
			dev_info(la9310_dev->dev, "V2H: Pkt rcd: 0x%x ", *ptr);
			ptr++;
		}
		#endif
		kfree_skb(skb_ptr);
	} else {
		dev_dbg(la9310_dev->dev, "V2H Callback: sk buffer is null\n");
	}
}

int v2h_callback_test_init(struct la9310_dev *la9310_dev)
{
	int ret = 0;

	dev_info(la9310_dev->dev, "V2H Callback registered\n");
	ret = register_v2h_callback(la9310_device_name, callback_func,
				     la9310_dev);

	return ret;
}

int v2h_callback_test_deinit(void)
{
	int ret = 0;

	ret = unregister_v2h_callback(la9310_device_name);

	return ret;
}
