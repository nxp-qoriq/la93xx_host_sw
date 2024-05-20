/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2021, 2024 NXP
 */

#include "la9310_base.h"
#include "la9310_sync_timing_device.h"

int sync_timing_device_load_fw(struct la9310_dev *la9310_dev,
			       char (*std_fw_list)[STD_FW_NAME_MAX_LENGTH],
			       uint32_t fw_count)
{
	int ret;
	int i;
	int offset = 0;
	struct la9310_mem_region_info *std_fw_region;
	struct la9310_sw_cmd_desc *cmd_desc;
	struct la9310_std_fwupgrade_data *cmd_data;

	std_fw_region = la9310_get_dma_region(la9310_dev,
					      LA9310_MEM_REGION_STD_FW);

	ret = la9310_dev_reserve_firmware(la9310_dev);
	if (ret < 0) {
		dev_err(la9310_dev->dev,
			"%s: can't reserve device for FW load\n", __func__);
		return ret;
	}

	cmd_desc = &(la9310_dev->hif->sw_cmd_desc);
	if (cmd_desc->status != LA9310_SW_CMD_STATUS_FREE) {
		dev_err(la9310_dev->dev, "sw cmd desc status is not free: %d\n",
			cmd_desc->status);
		return -1;
	}

	cmd_desc->cmd = LA9310_SW_CMD_STD_FW_UPGRADE;
	cmd_data = (struct la9310_std_fwupgrade_data *) cmd_desc->data;
	cmd_data->fwcount = fw_count;

	for (i = 0; i < fw_count; i++) {
		ret = la9310_udev_load_firmware(la9310_dev,
						std_fw_region->vaddr + offset,
						std_fw_region->size - offset,
						std_fw_list[i]);
		if (ret < 0) {
			dev_err(la9310_dev->dev, "%s: load_firmware request failed\n",
				__func__);
			return -1;
		}

		cmd_data->fwinfo[i].addr = std_fw_region->phys_addr + offset;
		cmd_data->fwinfo[i].size = la9310_dev->firmware_info.size;
		dev_info(la9310_dev->dev, "loading %s(0x%x) at %px, 0x%x\n",
			 std_fw_list[i], cmd_data->fwinfo[i].size,
			 std_fw_region->vaddr + offset,
			 cmd_data->fwinfo[i].addr);
		offset += la9310_dev->firmware_info.size;
	}

	la9310_dev_free_firmware(la9310_dev);

	return 0;
}
