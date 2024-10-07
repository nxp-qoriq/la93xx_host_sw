/* SPDX-License-Identifier: GPL-2.0
 * Copyright 2024 NXP
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/irqreturn.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/wait.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/swait.h>
#include <linux/delay.h>
#ifdef CONFIG_IMX8MM_THERMAL_MONITOR
#include <linux/imx8mm_thermal.h>
#endif
#include <linux/eventfd.h>
#include "la9310_base.h"
#include "la9310_tvd_ioctl.h"

#define TVD_DEVICE_NAME_LEN	16

#define MAX_MODEM               4
#define MAX_ATTEMPT 5
#define TMU_TRITSR0  0x1F80100
#define TMU_TRITSR_OFFSET_DIF 0x10
#define MAX_TEMP_MONITORING_SITE_ENABLED 2
#define TEMP_KELVIN_TO_CELSIUS(val)	( val & 0x1FF )

static dev_t tvd_dev_num;
static uint32_t tvd_dev_major;
static uint32_t tvd_dev_minor;
static struct class *la9310_tvd_dev_class;
static struct la9310_tvd_device_data *tvd_dev_data_g[MAX_MODEM];
struct tvd_dev *g_tvd_dev;

struct tvd_priv_data {
	int tvd_id;
	uint64_t tvd_count;
	int mtd_cur_temp;
	int ctd_cur_temp;
	int get_mtd_cur_temp;
	int get_ctd_cur_temp;
	int get_rtd_cur_temp;
};

struct tvd_dev {
	char name[TVD_DEVICE_NAME_LEN];
	struct tvd_priv_data tvd_priv_d[MAX_MODEM];
	struct la9310_dev *la9310_dev[MAX_MODEM];
};

/* TVD char Dev data holder */
struct la9310_tvd_device_data {
	struct tvd_dev *tvd_dev;
	struct cdev cdev;
};

char tvd_dev_name[TVD_DEVICE_NAME_LEN];

void mtd_get_temp(struct tvd_dev *tvd_dev, uint32_t tvdid, struct mtdTemp *mtd_temp)
{
	int retry = 0, valid = 0, index = 0, temp = 0, ret = 0;
	struct la9310_dev *la9310_dev = NULL;
	struct tvd_priv_data *tvd_priv_d = NULL;
	struct la9310_mem_region_info *ccsr_region = NULL;
	tvd_priv_d = &tvd_dev->tvd_priv_d[tvdid];
	la9310_dev = tvd_dev->la9310_dev[tvdid];
	ccsr_region = &la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR];

	for (index = 0;index < MAX_TEMP_MONITORING_SITE_ENABLED; index++)
	{
		for (retry =0; retry < MAX_ATTEMPT; retry++)
		{
			valid = readl(ccsr_region->vaddr +
					(TMU_TRITSR0 +
					 (index * TMU_TRITSR_OFFSET_DIF))) ;
			if (!(valid & 0x80000000))
				/*waiting here to let the site come out of busy state & retry*/
				udelay(1);
			else
				break;
		}
		if (retry == MAX_ATTEMPT) {
			dev_err(la9310_dev->dev, "\nSite %d is busy OR temp out of range !!!\n",
						index);
			temp = MTD_TEMP_INVALID;
			ret = -1;
		} else {
			temp = TEMP_KELVIN_TO_CELSIUS(valid);
		}
		switch (index) {
			case VSPA_TEMP:
				mtd_temp->vspa_temp = temp;
				break;
			case DCS_TEMP:
				mtd_temp->dcs_temp = temp;
				break;
		};

	}
}

static int la9310_tvd_dev_open(struct inode *inode, struct file *filp)
{
	struct la9310_tvd_device_data *tvd_dev = NULL;

	tvd_dev = container_of(inode->i_cdev,
				struct la9310_tvd_device_data,
				cdev);
	filp->private_data = tvd_dev;

	return 0;
}

static int la9310_tvd_dev_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static long la9310_tvd_dev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	struct tvd tvd_t = {0};
	struct tvd *tvd_ptr = &tvd_t;
	struct la9310_tvd_device_data *tvd_dev_data = NULL;
	struct tvd_dev *tvd_dev = NULL;
	struct mtdTemp mtd_temp;
	int temp = 0;

	(void)(temp);
	tvd_dev_data = (struct la9310_tvd_device_data *)filp->private_data;
	tvd_dev = tvd_dev_data->tvd_dev;

	switch (cmd) {
	case IOCTL_LA9310_TVD_MTD_GET_TEMP:
		ret = copy_from_user(&tvd_t, (struct tvd *)arg,
					sizeof(struct tvd));
		if (ret != 0)
			return -EFAULT;

		tvd_ptr = (struct tvd *)arg;
		mtd_get_temp(tvd_dev, tvd_ptr->tvdid, &mtd_temp);
		/* Write MTD temp value to userspace buffer */
		put_user(mtd_temp, &tvd_ptr->get_mtd_curr_temp);
		break;

#ifdef CONFIG_IMX8MM_THERMAL_MONITOR
	case IOCTL_LA9310_TVD_CTD_GET_TEMP:

		ret = copy_from_user(&tvd_t,
					(struct tvd *)arg,
					sizeof(struct tvd));
		if (ret != 0)
			return -EFAULT;

		tvd_ptr = (struct tvd *)arg;
		imx8mm_get_temp(&temp, 0);
		tvd_dev_data->tvd_dev->tvd_priv_d[tvd_ptr->tvdid].get_ctd_cur_temp = temp;

		/* Write CTD temp value to userspace buffer */
		put_user(tvd_dev_data->tvd_dev->tvd_priv_d[tvd_ptr->tvdid].get_ctd_cur_temp,
					&tvd_ptr->get_ctd_curr_temp);
		break;
#endif
	default:
		ret = -ENOTTY;
	}
	return ret;
}


/* TVD file_operations */
static const struct file_operations la9310_tvd_dev_fops = {
	.owner      = THIS_MODULE,
	.open       = la9310_tvd_dev_open,
	.release    = la9310_tvd_dev_release,
	.unlocked_ioctl = la9310_tvd_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = la9310_tvd_dev_ioctl,
#endif
};

static int create_tvd_cdevs(struct la9310_tvd_device_data **tvd_dev_data, int id)
{
	int ret = 0;

	cdev_init(&tvd_dev_data[id]->cdev, &la9310_tvd_dev_fops);
	tvd_dev_data[id]->cdev.ops = &la9310_tvd_dev_fops;
	tvd_dev_data[id]->cdev.owner = THIS_MODULE;

	/* Adding a device to the system */
	cdev_add(&tvd_dev_data[id]->cdev, MKDEV(tvd_dev_major, id), 1);
	if ((device_create(la9310_tvd_dev_class,
				NULL,
				MKDEV(tvd_dev_major, id),
				NULL,
				"%s%d", tvd_dev_name, id)) == NULL) {
		pr_err("%s: Cannot create the tvd device(%d)\n", __func__, id);
		ret = -1;
	}
	return ret;
}

int tvd_probe(struct la9310_dev *la9310_dev, int virq_count,
				struct virq_evt_map *virq_map)
{
	struct tvd_dev *tvd_dev = g_tvd_dev;
	struct la9310_tvd_device_data *tvd_dev_data[MAX_MODEM];
	int i;
	int ret = 0;

	dev_dbg(la9310_dev->dev, "In probe function\n");

	i = la9310_dev->id;
	tvd_dev->la9310_dev[i] = la9310_dev;

	tvd_dev_data[i] = kmalloc(sizeof(struct la9310_tvd_device_data),
					GFP_KERNEL);
	if (tvd_dev_data[i] == NULL)
		return -ENOMEM;

	tvd_dev->tvd_priv_d[i].tvd_id = i;
	tvd_dev_data[i]->tvd_dev = tvd_dev;
	tvd_dev_data_g[i] = tvd_dev_data[i];

	ret = create_tvd_cdevs(tvd_dev_data, i);
	if (ret < 0) {
		dev_err(la9310_dev->dev, "TVD Failed to create chardevs");
		goto err;
	}

	la9310_dev->tvd_priv = tvd_dev;

	return 0;
err:
	kfree(tvd_dev_data[i]);
	tvd_dev_data_g[i] = NULL;
	la9310_dev->tvd_priv = NULL;

	return ret;
}

int tvd_remove(struct la9310_dev *la9310_dev)
{
	struct tvd_dev *tvd_dev;
	int tvd_id = la9310_dev->id;

	tvd_dev = la9310_dev->tvd_priv;

	if (!tvd_dev)
		return 0;

	device_destroy(la9310_tvd_dev_class, MKDEV(tvd_dev_major, tvd_id));
	cdev_del(&tvd_dev_data_g[tvd_id]->cdev);
	kfree(tvd_dev_data_g[tvd_id]);
	tvd_dev_data_g[tvd_id] = NULL;

	la9310_dev->tvd_priv = NULL;
	return 0;
}

int tvd_init(void)
{
	int ret = -1;
	struct tvd_dev *tvd_dev = NULL;

	pr_debug("%s:TVD init called\n", __func__);

	sprintf(tvd_dev_name, "%s%s",
		LA9310_DEV_NAME_PREFIX, LA9310_TVD_DEV_NAME_PREFIX);
	/*Allocating chardev region and assigning Major number */
	ret = alloc_chrdev_region(&tvd_dev_num,
					tvd_dev_minor,
					MAX_MODEM,
					tvd_dev_name);
	if (ret < 0) {
		pr_err("%s: Failed in getting major number\n",
				 __func__);
		return ret;
	}

	/* Device Major number */
	tvd_dev_major = MAJOR(tvd_dev_num);

	tvd_dev = kmalloc(sizeof(struct tvd_dev), GFP_KERNEL);
	if (!tvd_dev) {
		pr_err("%s:Memory allocation failure for tvd_dev\n", __func__);
		goto err;
	}
	memset(tvd_dev, 0, sizeof(struct tvd_dev));

	/* sysfs class creation */
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION( 6, 4, 0 ) )
	la9310_tvd_dev_class = class_create( tvd_dev_name);
#else
	la9310_tvd_dev_class = class_create(THIS_MODULE, tvd_dev_name);
#endif
	if (la9310_tvd_dev_class == NULL) {
		pr_err("%s:Cannot allocate major number\n", __func__);
		ret = -1;
		goto out;
	}

	g_tvd_dev = tvd_dev;
	return ret;
out:
	class_destroy(la9310_tvd_dev_class);
	kfree(tvd_dev);
err:
	unregister_chrdev_region(tvd_dev_num, MAX_MODEM);

	return ret;
}
EXPORT_SYMBOL_GPL(tvd_init);

int tvd_exit(void)
{

	class_destroy(la9310_tvd_dev_class);
	kfree(g_tvd_dev);
	unregister_chrdev_region(tvd_dev_num, MAX_MODEM);

	return 0;
}
EXPORT_SYMBOL_GPL(tvd_exit);
