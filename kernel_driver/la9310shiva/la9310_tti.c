/* SPDX-License-Identifier: GPL-2.0
 * Copyright 2024 NXP
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fdtable.h>
#include <linux/rcupdate.h>
#include <linux/eventfd.h>
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
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include "la9310_tti_ioctl.h"
#include "la9310_base.h"

#define SIGNAL_TO_USERSPACE	1
char tti_dev_name[20];

static struct tti_priv *tti_priv_g;

/* TTI IRQ Handler */
static irqreturn_t tti_irq_handler(int irq, void *data)
{
	struct tti_priv *tti_priv_t = (struct tti_priv *)data;

	pr_info("%s: tti IRQ handler invoked\n", __func__);
	/* Send signal to User space */
	if (tti_priv_t) {
		raw_spin_lock(&tti_priv_t->wq_lock);
		swake_up_all_locked(&tti_priv_t->tti_wq);
		raw_spin_unlock(&tti_priv_t->wq_lock);
		if (tti_priv_t->evt_fd_ctxt) {
			eventfd_signal(tti_priv_t->evt_fd_ctxt,
				SIGNAL_TO_USERSPACE);
		}
	}
	return IRQ_HANDLED;
}

void tti_irq_api(void)
{
	/* Send signal to User space */
	if (tti_priv_g) {
		raw_spin_lock(&tti_priv_g->wq_lock);
		swake_up_all_locked(&tti_priv_g->tti_wq);
		raw_spin_unlock(&tti_priv_g->wq_lock);
		if (tti_priv_g->evt_fd_ctxt) {
			eventfd_signal(tti_priv_g->evt_fd_ctxt,
				SIGNAL_TO_USERSPACE);
		}
	}
	return;
}
EXPORT_SYMBOL(tti_irq_api);

void set_tti_irq_status(u8 irq_status) {
	tti_priv_g->tti_irq_status = irq_status;
}
EXPORT_SYMBOL(set_tti_irq_status);

int tti_register_irq(struct tti_priv *tti_priv_t, struct tti *tti_t)
{
	int ret = 0;
	int tti_interrupt_flags = IRQF_NO_THREAD | IRQF_TRIGGER_RISING;
	struct task_struct *userspace_task = NULL;
	struct file *efd_file = NULL;

	tti_priv_t->evt_fd_ctxt = NULL;

	if (tti_priv_t->irq != 0) {
		if (tti_priv_t->tti_irq_status == 0) {
		/* IRQ request */
#ifndef RFNM
			ret = request_irq(tti_priv_t->irq, tti_irq_handler,
					tti_interrupt_flags, "tti_handler", tti_priv_t);
			if (ret < 0) {
				pr_err("%s request irq err - %d\n",
							__func__, ret);
				goto err;
			} else {
				tti_priv_t->tti_irq_status = 1;
			}
#endif
		} else {
			pr_err("%s TTI IRQ %d is busy\n",
				__func__, tti_priv_t->irq);
		}
	} else {
		pr_err("%s IRQ is invalid\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	if (tti_t->tti_eventfd > 0) {
		userspace_task = current;
		rcu_read_lock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
		efd_file = files_lookup_fd_rcu(userspace_task->files,
				tti_t->tti_eventfd);
#else
		efd_file = fcheck_files(userspace_task->files,
				tti_t->tti_eventfd);
#endif
		rcu_read_unlock();
		tti_priv_t->evt_fd_ctxt = eventfd_ctx_fileget(efd_file);
	}
err:
	return ret;
}

void tti_deregister_irq(struct tti_priv *tti_priv_t, struct tti *tti_t)
{
	if (tti_priv_t != NULL) {
		/* IRQ request */
		free_irq(tti_priv_t->irq, tti_priv_t);
		tti_priv_t->tti_irq_status = 0;

		if (tti_priv_t->evt_fd_ctxt) {
			eventfd_ctx_put(tti_priv_t->evt_fd_ctxt);
			tti_priv_t->evt_fd_ctxt = NULL;
		}
	}
}

static int la9310_tti_dev_open(struct inode *inode, struct file *filp)
{
	struct la9310_tti_device_data *tti_dev = NULL;

	tti_dev = container_of(inode->i_cdev, struct la9310_tti_device_data, cdev);
	filp->private_data = tti_dev;
	return 0;
}

static int la9310_tti_dev_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static ssize_t la9310_tti_dev_read(struct file *filp, char __user *buf,
				size_t count, loff_t *offset)
{
	int rc = 0;
	DECLARE_SWAITQUEUE(wait);
	struct la9310_tti_device_data *tti_dev = NULL;

	tti_dev = filp->private_data;
	raw_spin_lock(&tti_dev->tti_dev->wq_lock);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	prepare_to_swait_exclusive(&tti_dev->tti_dev->tti_wq, &wait,
						TASK_INTERRUPTIBLE);
#else
	prepare_to_swait(&tti_dev->tti_dev->tti_wq, &wait,
						TASK_INTERRUPTIBLE);
#endif
	raw_spin_unlock(&tti_dev->tti_dev->wq_lock);

	/*Now wait here, tti notificaion will wakeup*/
	schedule();

	raw_spin_lock(&tti_dev->tti_dev->wq_lock);
	finish_swait(&tti_dev->tti_dev->tti_wq, &wait);
	raw_spin_unlock(&tti_dev->tti_dev->wq_lock);

	rc = put_user(tti_dev->tti_dev->tti_count, (int *)buf);
	if (!rc)
		rc = sizeof(tti_dev->tti_dev->tti_count);
	return rc;
}

static long la9310_tti_dev_ioctl(struct file *filp, unsigned int cmd,
			unsigned long arg)
{
	int ret = 0;
	struct tti tti_t = {0};
	struct la9310_tti_device_data *tti_dev = NULL;
	struct tti_priv *tti_device = NULL;

	tti_dev = (struct la9310_tti_device_data *)filp->private_data;
	tti_device = tti_dev->tti_dev;

	switch (cmd) {
	case IOCTL_LA9310_MODEM_TTI_REGISTER:
		ret = copy_from_user(&tti_t, (struct tti *)arg,
						sizeof(struct tti));
		if (ret != 0)
			return -EFAULT;
		ret = tti_register_irq(tti_device, &tti_t);
		break;

	case IOCTL_LA9310_MODEM_TTI_DEREGISTER:
		ret = copy_from_user(&tti_t, (struct tti *)arg,
						sizeof(struct tti));
		if (ret != 0)
			return -EFAULT;
		tti_deregister_irq(tti_device, &tti_t);
		break;

	default:
		ret = -ENOTTY;
	}
	return ret;
}


/* TTI initialize file_operations */
static const struct file_operations la9310_tti_dev_fops = {
	.owner		= THIS_MODULE,
	.open		= la9310_tti_dev_open,
	.release	= la9310_tti_dev_release,
	.read		= la9310_tti_dev_read,
	.unlocked_ioctl = la9310_tti_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = la9310_tti_dev_ioctl,
#endif
};

static struct class *la9310_tti_dev_class;
static dev_t tti_dev_number;
static int tti_dev_major;
static struct la9310_tti_device_data *tti_dev_data;

ssize_t
tti_device_dump(int id, char *buf)
{
	struct tti_priv *priv;

	if (tti_dev_data[id].tti_dev == NULL)
		return 0;
	priv = tti_dev_data[id].tti_dev;

	sprintf(&buf[strlen(buf)],
		" TTI:%s%d irq %d irq status=%d\n",
		tti_dev_name, id, priv->irq,
		priv->tti_irq_status);
	return 0;
}

void tti_dev_stop(struct la9310_dev *dev)
{
	struct tti_priv *priv;
	int j;

	j = dev->id;

	if (tti_dev_data[j].tti_dev == NULL)
		return;
	priv = tti_dev_data[j].tti_dev;
	if (priv && priv->tti_irq_status) {
		free_irq(priv->irq, priv);
		priv->tti_irq_status = 0;
	}
	cdev_del(&tti_dev_data[j].cdev);
	device_destroy(la9310_tti_dev_class, MKDEV(tti_dev_major,
			j));
	tti_priv_g = NULL;
	kfree(tti_dev_data[j].tti_dev);
	tti_dev_data[j].tti_dev = NULL;
}

int tti_dev_start(struct la9310_dev *dev)
{
	struct device_node *la9310_tti;
	struct tti_priv *tti_dev = NULL;
	int i, j, ret, tti_gpio;
	j = dev->id;

#ifndef RFNM
	la9310_tti = of_find_node_by_name(NULL , "la9310_tti");
	if (!la9310_tti) {
		dev_err(dev->dev, "TTI node not available in the dtb.\n");
		return -ENODATA;
	}

	tti_gpio = of_get_named_gpio(la9310_tti , "la9310-tti-gpio", i);
	if (!tti_gpio) {
		dev_err(dev->dev, "la9310-tti-gpio %d not found in the dtb\n",
				i);
		return -ENODATA;
	}

	ret = gpio_request(tti_gpio, "tti interrupt gpio");
	if (ret) {
		dev_err(dev->dev, "Can't request gpio %d, error: %d\n",
				tti_gpio, ret);
		return ret;
	}

	ret = gpio_direction_input(tti_gpio);
	if (ret < 0) {
		dev_err(dev->dev, "Can't configure gpio %d\n",	tti_gpio);
		gpio_free(tti_gpio);
		return ret;
	}
#endif

	tti_dev = kmalloc(sizeof(struct tti_priv), GFP_KERNEL);
	if (tti_dev == NULL)
		return -ENOMEM;
#ifndef RFNM
	tti_dev->irq = gpio_to_irq(tti_gpio);
	if (tti_dev->irq < 0) {
		dev_dbg(dev->dev,
			"ttidev: TTI IRQ not available%d\n", j+1);
		kfree(tti_dev);
		goto err;
	}
#endif

	tti_dev->tti_irq_status = 0;
	/*simple wait queue init*/
	init_swait_queue_head(&tti_dev->tti_wq);
	/*raw spinlock init for TTI module*/
	raw_spin_lock_init(&tti_dev->wq_lock);

	tti_dev_data[j].tti_dev = tti_dev;

	if (device_create(la9310_tti_dev_class, NULL,
			MKDEV(tti_dev_major, j),
			NULL, "%s%d", tti_dev_name, j) == NULL) {
		dev_dbg(dev->dev,
			"tti device_create failed :%d\n", j);
		kfree(tti_dev);
		goto err;
	}

	cdev_init(&tti_dev_data[j].cdev, &la9310_tti_dev_fops);
	tti_dev_data[j].cdev.ops = &la9310_tti_dev_fops;
	tti_dev_data[j].cdev.owner = THIS_MODULE;
	/* Adding a device to the system:i-Minor number of new device*/
	cdev_add(&tti_dev_data[j].cdev, MKDEV(tti_dev_major,
				j), 1);

	tti_priv_g = tti_dev;

	return 0;

	err:
		return -ENODEV;
}

int init_tti_dev(void)
{
	int err;

	sprintf(tti_dev_name, "%s%s",
		LA9310_DEV_NAME_PREFIX, LA9310_TTI_DEVNAME_PREFIX);
	/*Allocating chardev region and assigning Major number*/
	err = alloc_chrdev_region(&tti_dev_number, 0,
				  MAX_MODEM,
				  tti_dev_name);
	/* Device Major number*/
	tti_dev_major = MAJOR(tti_dev_number);
	/*sysfs class creation */
	la9310_tti_dev_class = class_create(THIS_MODULE, tti_dev_name);

	tti_dev_data = kmalloc(MAX_MODEM * sizeof(*tti_dev_data), GFP_KERNEL);
	if (tti_dev_data == NULL) {
		pr_err("TTI device data array alloc failed\n");
		err = -ENOMEM;
		goto out;
	}
out:
	return err;
}
EXPORT_SYMBOL_GPL(init_tti_dev);

void remove_tti_dev(void)
{

	class_destroy(la9310_tti_dev_class);
	la9310_tti_dev_class = NULL;
	unregister_chrdev_region(tti_dev_number, MAX_MODEM);

	kfree(tti_dev_data);
}
EXPORT_SYMBOL_GPL(remove_tti_dev);
