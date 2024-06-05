/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
* Copyright 2024 NXP
*
*/

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/unistd.h>
#include <linux/mm.h>
#include <linux/memblock.h>
#include <linux/slab.h>
#include <linux/mman.h>
#include <linux/fdtable.h>

#include "la9310_base.h"
#include <la93xx_ipc_ioctl.h>
#include <la9310_host_if.h>

#include <linux/module.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/rcupdate.h>
#include <linux/eventfd.h>
#include <linux/irqreturn.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>

#include <la9310_modinfo.h>

extern struct list_head pcidev_list;

extern int modem_share_buf_size;
extern int modem_host_data_size;
extern int modem_rf_data_size;

#define la9310_MODINFO_MINOR_CNT 1
#define LA9310_BOARD_NAME "imx8mp-rfnm"

static int la9310_modinfo_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int rc;
	struct la9310_dev *la9310_dev = filp->private_data;
	phys_addr_t offset = (phys_addr_t)vma->vm_pgoff << PAGE_SHIFT;
	size_t size = vma->vm_end - vma->vm_start;


	if (!la9310_dev) {
		return -EINVAL;
	}

	dev_dbg(la9310_dev->dev, "request to mmap %llx:%lx\n", offset, size);

	if ((offset >= scratch_buf_phys_addr) &&
		((offset + size) <= scratch_buf_phys_addr +
					scratch_buf_size)) {
		vma->vm_page_prot = pgprot_cached(vma->vm_page_prot);
		dev_dbg(la9310_dev->dev,
			"request to mmap %llx:%lx marked cacheable\n",
			offset, size);
	} else
		return -EINVAL;

	rc = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size,
				vma->vm_page_prot);
	return rc;
}

void la9310_modinfo_get(struct la9310_dev *la9310_dev, modinfo_t *mi)
{
	int32_t idx;
	struct la9310_mem_region_info *ep_buf;
	int name_len = 2;
	char *brd_name = "NA";
	struct device_node *root;

	sprintf(mi->name, "%s", la9310_dev->name);
	mi->id = la9310_dev->id;

	root = of_find_node_by_path("/");
	if (root) {
		if (of_machine_is_compatible("fsl,imx8mp-evk") ||
			of_machine_is_compatible("fsl,imx8mp") ||
			of_machine_is_compatible("fsl,imx8dxl-evk") ||
			of_machine_is_compatible("fsl,imx8dxl")) {
			brd_name = (char *)of_get_property(root, "model", &name_len);
			if (brd_name)
				strncpy(mi->board_name, brd_name, name_len);
		}
		else {
			strncpy(mi->board_name, brd_name, name_len);
		}
	}
	else {
		strncpy(mi->board_name, brd_name, name_len);
	}
	of_node_put(root);

	sprintf(mi->pci_addr, "%s", g_la9310_global[0].dev_name);

	mi->ccsr.host_phy_addr = la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR].phys_addr;
	mi->ccsr.size = la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR].size;

	mi->tcml.host_phy_addr = la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].phys_addr;
	mi->tcml.size = la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].size;

	mi->tcmu.host_phy_addr = la9310_dev->mem_regions[LA9310_MEM_REGION_TCMU].phys_addr;
	mi->tcmu.size = la9310_dev->mem_regions[LA9310_MEM_REGION_TCMU].size;

	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_VSPA_OVERLAY);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->ov.host_phy_addr = ep_buf->phys_addr;
	mi->ov.size = ep_buf->size;

	/* VSPA */
	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_VSPA);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->vspa.host_phy_addr = ep_buf->phys_addr;
	mi->vspa.size = ep_buf->size;

	/* FW*/
	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_FW);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->fw.host_phy_addr = ep_buf->phys_addr;
	mi->fw.size = ep_buf->size;

	/* LA9310 LOG buffer */
	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_DBG_LOG);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->dbg.host_phy_addr = ep_buf->phys_addr;
	mi->dbg.size = ep_buf->size;

	/* IQ Data samples*/
	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_IQ_SAMPLES);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->iqr.host_phy_addr = ep_buf->phys_addr;
	mi->iqr.size = ep_buf->size;

	/*  NLM Operations */
	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_NLM_OPS);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->nlmops.host_phy_addr = ep_buf->phys_addr;
	mi->nlmops.size = ep_buf->size;

	idx = LA9310_SUBDRV_DMA_REGION_IDX(LA9310_MEM_REGION_STD_FW);
	ep_buf = &la9310_dev->dma_info.ep_bufs[idx];
	mi->stdfw.host_phy_addr = ep_buf->phys_addr;
	mi->stdfw.size = ep_buf->size;

	mi->hif.host_phy_addr = la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].phys_addr + LA9310_EP_HIF_OFFSET;
	mi->hif.size = sizeof(struct la9310_hif);

	mi->pciwin.host_phy_addr = la9310_dev->pci_outbound_win_start_addr;
	mi->pciwin.size = (uint32_t)(la9310_dev->pci_outbound_win_limit - la9310_dev->pci_outbound_win_start_addr);

	mi->scratchbuf.host_phy_addr = scratch_buf_phys_addr;
	mi->scratchbuf.size = scratch_buf_size;

	mi->dac_mask = dac_mask;
	mi->adc_mask = adc_mask;
	mi->adc_rate_mask = adc_rate_mask;
	mi->dac_rate_mask = dac_rate_mask;

	mi->iqflood.modem_phy_addr = LA9310_IQFLOOD_PHYS_ADDR;
	mi->iqflood.host_phy_addr = iq_mem_addr;
	mi->iqflood.size = iq_mem_size;
}

static long la9310_modinfo_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	int32_t ret = 0;
	struct la9310_dev *la9310_dev = filp->private_data;
	struct la9310_host_stats *host_stats;
	int list_stats_len = 0, stats_len = 0;
	char *buf;

	switch (cmd) {
	case IOCTL_LA93XX_MODINFO_GET:
	{
		modinfo_t *mil_user = (modinfo_t *)arg;
		modinfo_t mil;
		modinfo_t *mi = &mil;

		ret = copy_from_user(&mil, (modinfo_t *)arg, sizeof(modinfo_t));
		if (ret != 0) {
			dev_err(NULL, "la9310modinfo: copyfromuser failed\n");
			return -EFAULT;
		}

		la9310_modinfo_get(la9310_dev, mi);

		ret = copy_to_user(mil_user, &mil, sizeof(modinfo_t));
		if (ret != 0) {
			dev_err(NULL, "la9310modinfo: copytouser failed\n");
			return -EFAULT;
		}
	}
	break;
	case IOCTL_LA93XX_MODINFO_GET_STATS:
	{
		modinfo_s *mil_user_s = (modinfo_s *)arg;
		modinfo_s mil_s;
		modinfo_s *mi_s = &mil_s;
		ret = copy_from_user(&mil_s, (modinfo_s *)arg, sizeof(modinfo_s));
		if (ret != 0) {
			dev_err(NULL, "la9310modinfo: copyfromuser failed\n");
			return -EFAULT;
		}

		buf = mi_s->target_stat;

		list_for_each_entry(host_stats, &la9310_dev->host_stats.list, list) {
			if (stats_len > PAGE_SIZE) {
				dev_err(la9310_dev->dev,
				"LA9310 host stats > sysfs max permisible");
				return stats_len;
			}
			list_stats_len =
				host_stats->stats_ops.la9310_show_stats(host_stats->
						stats_ops.
						stats_args,
						buf,
						la9310_dev);
			stats_len += list_stats_len;
			buf += list_stats_len;
		}

		ret = copy_to_user(mil_user_s, &mil_s, sizeof(modinfo_s));
		if (ret != 0) {
			dev_err(NULL, "la9310modinfo: copytouser failed\n");
			return -EFAULT;
		}
	}
	break;
	default:
		dev_err(NULL, "INVALID ioctl for la9310modinfo\n");
		return -EINVAL;
	}

return 0;
}

static int la9310_modinfo_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static int la9310_modinfo_open(struct inode *inode, struct file *filp)
{
	struct la9310_dev *la9310_dev;
	const char *file_name = file_dentry(filp)->d_iname;

	la9310_dev = get_la9310_dev_byname(file_name);

	if (!la9310_dev) {
		return -EINVAL;
	}

	filp->private_data = (void *)la9310_dev;

	return 0;
}

static const struct file_operations la9310_modinfo_fops = {
	.owner		= THIS_MODULE,
	.open              = la9310_modinfo_open,
	.release           = la9310_modinfo_release,
	.mmap              = la9310_modinfo_mmap,
	.unlocked_ioctl = la9310_modinfo_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = la9310_modinfo_ioctl,
#endif
};

static struct miscdevice la9310_modinfo_miscdev[MAX_MODEM_INSTANCES];
int la9310_modinfo_init(struct la9310_dev *la9310_dev)
{
	int rc = -1;
	struct miscdevice *miscdev;

	if (la9310_dev->id >= MAX_MODEM_INSTANCES) {
		dev_err(la9310_dev->dev, "Invalid modem id : %d\n", la9310_dev->id);
		return rc;
	}

	miscdev = &la9310_modinfo_miscdev[la9310_dev->id];

	miscdev->name = la9310_dev->name;
	miscdev->fops = &la9310_modinfo_fops;
	miscdev->minor = MISC_DYNAMIC_MINOR;

	rc = misc_register(miscdev);

	if (rc)
		dev_err(la9310_dev->dev,
			"la9310_modinfo: failed to register misc device\n");
	else
		dev_dbg(la9310_dev->dev,
			"la9310_modinfo: misc driver created with minor :%d\n",
			miscdev->minor);

	return rc;
}
EXPORT_SYMBOL_GPL(la9310_modinfo_init);


int la9310_modinfo_exit(struct la9310_dev *la9310_dev)
{
	if (la9310_dev->id >= MAX_MODEM_INSTANCES) {
		dev_err(la9310_dev->dev, "Invalid modem id : %d\n", la9310_dev->id);
		return -1;
	}
	misc_deregister(&la9310_modinfo_miscdev[la9310_dev->id]);
	return 0;
}
EXPORT_SYMBOL_GPL(la9310_modinfo_exit);
