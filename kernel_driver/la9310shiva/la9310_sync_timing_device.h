/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2021 NXP
 */

#ifndef __LA9310_SYNC_TIMING_DEVICE_H__
#define __LA9310_SYNC_TIMING_DEVICE_H__

#define STD_PROD_FW_NAME	"prod_fw.boot.bin"
#define STD_USER_CONFIG_NAME	"user_config.boot.bin"
#define STD_MAX_FW_COUNT	(2)
#define STD_FW_NAME_MAX_LENGTH	(100)
#define STD_WAIT		(100)

int sync_timing_device_load_fw(struct la9310_dev *la9310_dev,
		char (*std_fw_list)[STD_FW_NAME_MAX_LENGTH],
		uint32_t fw_count);

#endif	/* __LA9310_SYNC_TIMING_DEVICE_H__ */
