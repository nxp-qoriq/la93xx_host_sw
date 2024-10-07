/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2021-2024 NXP
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
#include <linux/poll.h>
#include <linux/swait.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include "la9310_wdog.h"
#include "la9310_base.h"
#include <la9310_wdog_ioctl.h>

#define WDOG_DEVICE_NAME_LEN 16
char wdog_dev_name[WDOG_DEVICE_NAME_LEN];

struct wdog_priv {
	int irq;
	int irq_status_flag;
	int gpio;
	int gpio_hrst;
	int gpio_bootstrap;
	int wdog_id;
	int wdog_modem_status;
	int domain_nr;
	int wdog_event_status;
	uint64_t wdog_count;
	raw_spinlock_t wdog_wq_lock;
	wait_queue_head_t wdog_wq;
};

struct wdog_dev {
	char name[WDOG_DEVICE_NAME_LEN];
	struct wdog_priv wdog_pd[WDOG_ID_MAX];
	struct cdev cdev[WDOG_ID_MAX];
	int wdog_dev_major;
	struct class *wdog_class;
	dev_t wdog_dev_number;
};

struct wdog_dev *wdog_gdev;

static irqreturn_t wdog_irq_handler(int irq, void *data)
{
	struct wdog_priv *wdog_pd = (struct wdog_priv *)data;

	if (wdog_pd) {
		raw_spin_lock(&wdog_pd->wdog_wq_lock);
		wdog_pd->wdog_modem_status = WDOG_MODEM_NOT_READY;
		wdog_pd->wdog_event_status = 1;
		wake_up(&wdog_pd->wdog_wq);
		raw_spin_unlock(&wdog_pd->wdog_wq_lock);
	}

	return IRQ_HANDLED;
}

static int wdog_register_irq(struct wdog_priv *wdog_pd, struct wdog *wdog)
{
	int ret = 0;
	char wdog_irq_name[22];

	(void)wdog;

	wdog_pd->wdog_event_status = 0;
	wdog_pd->irq = get_wdog_msi(wdog_pd->wdog_id);

	sprintf(wdog_irq_name, "%s%d", "la9310wdog_handler", wdog_pd->wdog_id);

	if (wdog_pd->irq != 0) {
		if (wdog_pd->irq_status_flag == 0) {
			/* IRQ request */
			ret = request_irq(wdog_pd->irq, wdog_irq_handler,
					  IRQF_NO_THREAD | IRQF_TRIGGER_RISING,
					  wdog_irq_name,
					  wdog_pd);
			if (ret < 0) {
				pr_err("%s request irq err - %d\n",
				       __func__, ret);
				goto err;
			}
			wdog_pd->irq_status_flag = 1;

		} else
			pr_info("%s WDOG IRQ %d is busy\n",
				__func__, wdog_pd->irq);
	} else {
		pr_err("%s IRQ is invalid\n", __func__);
		ret = -EINVAL;
	}
err:
	return ret;
}

static void wdog_deregister_irq(struct wdog_priv *wdog_pd, struct wdog *wdog)
{
	(void)wdog;
	if (wdog_pd != NULL)
		if (wdog_pd->irq_status_flag != 0) {
			wdog_pd->irq_status_flag = 0;
			free_irq(wdog_pd->irq, wdog_pd);
		}
}

static int wdog_gpio_config(struct wdog_priv *wdog_pd)
{
	int ret;

	ret = gpio_request(wdog_pd->gpio,
			   "watchdog modem reset");
	if (ret) {
		pr_err("%s: Can't request gpio %d, error: %d\n", __func__,
		       wdog_pd->gpio, ret);
		return ret;
	}

	ret = gpio_direction_output(wdog_pd->gpio, 1);
	if (ret < 0) {
		pr_err("%s: Can't configure gpio %d\n", __func__,
		       wdog_pd->gpio);
		gpio_free(wdog_pd->gpio);
		goto err;
	}

	if (wdog_pd->gpio_hrst != 0) {
		ret = gpio_request(wdog_pd->gpio_hrst,
				   "watchdog modem reset");
		if (ret) {
			pr_err("%s: Can't request gpio %d, error: %d\n", __func__,
			       wdog_pd->gpio_hrst, ret);
			goto err;
		}

		ret = gpio_direction_output(wdog_pd->gpio_hrst, 1);
		if (ret < 0) {
			pr_err("%s: Can't configure gpio %d\n", __func__,
			       wdog_pd->gpio_hrst);
			gpio_free(wdog_pd->gpio_hrst);
			goto err1;
		}
	}

	if (wdog_pd->gpio_bootstrap != 0) {
		ret = gpio_request(wdog_pd->gpio_bootstrap,
				   "watchdog modem config reset");
		if (ret) {
			pr_err("%s: Can't request gpio %d, error: %d\n", __func__,
			       wdog_pd->gpio_bootstrap, ret);
			goto err;
		}

		ret = gpio_direction_output(wdog_pd->gpio_bootstrap, 1);
		if (ret < 0) {
			pr_err("%s: Can't configure gpio %d\n", __func__,
			       wdog_pd->gpio_hrst);
			gpio_free(wdog_pd->gpio_bootstrap);
			goto err2;
		}

	}
err2:
	gpio_free(wdog_pd->gpio_bootstrap);
err1:
	gpio_free(wdog_pd->gpio_hrst);
err:
	gpio_free(wdog_pd->gpio);
	return ret;
}

static void wdog_reset_modem(struct wdog_priv *wdog_pd, struct wdog *wdog)
{
	(void)wdog;
	pr_info("%s: Resetting LA9310 Modem\n", __func__);
	gpio_set_value_cansleep(wdog_pd->gpio, 0);
	if (wdog_pd->gpio_hrst != 0)
		gpio_set_value_cansleep(wdog_pd->gpio_hrst, 0);
	if (wdog_pd->gpio_bootstrap != 0)
		gpio_set_value_cansleep(wdog_pd->gpio_bootstrap, 0);
	mdelay(1);
	gpio_set_value_cansleep(wdog_pd->gpio, 1);
	if (wdog_pd->gpio_hrst != 0)
		gpio_set_value_cansleep(wdog_pd->gpio_hrst, 1);
	if (wdog_pd->gpio_bootstrap != 0)
		gpio_set_value_cansleep(wdog_pd->gpio_bootstrap, 1);
}

int wdog_set_modem_status(int wdog_id, int status)
{
	struct wdog_dev *wdog_dev = wdog_gdev;
	struct wdog_priv *wdog_pd;

	if (wdog_id >= WDOG_ID_MAX)
		return -ENODEV;

	if (wdog_dev == NULL)
		return -ENODEV;

	wdog_pd = &wdog_dev->wdog_pd[wdog_id];
	wdog_pd->wdog_modem_status = status;

	return 0;
}

int wdog_set_pci_domain_nr(int wdog_id, int domain_nr)
{
	struct wdog_dev *wdog_dev = wdog_gdev;
	struct wdog_priv *wdog_pd;

	if (wdog_id >= WDOG_ID_MAX)
		return -ENODEV;

	if (wdog_dev == NULL)
		return -ENODEV;

	wdog_pd = &wdog_dev->wdog_pd[wdog_id];
	wdog_pd->domain_nr = domain_nr;

	return 0;
}

static int wdog_dev_open(struct inode *inode, struct file *filp)
{
	filp->private_data = &wdog_gdev->wdog_pd[MINOR(inode->i_rdev)];
	return 0;
}

static int wdog_dev_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;
	return 0;
}

static ssize_t wdog_dev_read(struct file *filp, char __user *buf,
		size_t count, loff_t *offset)
{
	int rc = 0;
	return rc;
}

static unsigned int wdog_dev_poll(struct file *filp,
		struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	struct wdog_priv *wdog_pd = (struct wdog_priv *)filp->private_data;

	poll_wait(filp, &wdog_pd->wdog_wq, wait);
	if (wdog_pd->wdog_event_status) {
		wdog_pd->wdog_event_status = 0;
		mask |= (POLLIN | POLLRDNORM);
	}

	return mask;
}

static long wdog_dev_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;
	struct wdog wdog = {0};
	struct wdog_priv *wdog_pd = (struct wdog_priv *)filp->private_data;

	switch (cmd) {
	case IOCTL_LA9310_MODEM_WDOG_REGISTER:
		ret = copy_from_user(&wdog, (struct wdog *)arg,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;
		ret = wdog_register_irq(wdog_pd, &wdog);
		break;

	case IOCTL_LA9310_MODEM_WDOG_DEREGISTER:
		ret = copy_from_user(&wdog, (struct wdog *)arg,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;
		wdog_deregister_irq(wdog_pd, &wdog);
		break;

	case IOCTL_LA9310_MODEM_WDOG_RESET:
		ret = copy_from_user(&wdog, (struct wdog *)arg,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;

		wdog_reset_modem(wdog_pd, &wdog);
		break;

	case IOCTL_LA9310_MODEM_WDOG_GET_STATUS:
		ret = copy_from_user(&wdog, (struct wdog *)arg,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;

		wdog.wdog_modem_status = wdog_pd->wdog_modem_status;

		ret = copy_to_user((struct wdog *)arg,
				&wdog,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;
		break;

	case IOCTL_LA9310_MODEM_WDOG_GET_DOMAIN:
		ret = copy_from_user(&wdog, (struct wdog *)arg,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;

		wdog.domain_nr = wdog_pd->domain_nr;

		ret = copy_to_user((struct wdog *)arg,
				&wdog,
				sizeof(struct wdog));
		if (ret != 0)
			return -EFAULT;
		break;

	default:
		ret = -ENOTTY;
	}
	return ret;
}

static const struct file_operations wdog_dev_fops = {
	.owner      = THIS_MODULE,
	.open       = wdog_dev_open,
	.release    = wdog_dev_release,
	.read       = wdog_dev_read,
	.poll       = wdog_dev_poll,
	.unlocked_ioctl = wdog_dev_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = wdog_dev_ioctl,
#endif
};

static int create_wdog_cdevs(struct wdog_dev *wdog_dev)
{
	int i, ctr;
	int ret;

	sprintf(wdog_dev_name, "%s%s",
		LA9310_DEV_NAME_PREFIX, LA9310_WDOG_DEVNAME_PREFIX);
	if (alloc_chrdev_region(&(wdog_dev->wdog_dev_number),
				0,
				WDOG_ID_MAX,
				wdog_dev_name) < 0) {
		pr_err("%s: Cannot allocate major number\n",
		       __func__);
		goto err;
	}

	wdog_dev->wdog_dev_major = MAJOR(wdog_dev->wdog_dev_number);
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION( 6, 4, 0 ) )
	wdog_dev->wdog_class = class_create(wdog_dev_name);
#else
	wdog_dev->wdog_class = class_create(THIS_MODULE, wdog_dev_name);
#endif
	if (wdog_dev->wdog_class == NULL) {
		pr_err("%s:Cannot allocate major number\n",
		       __func__);
		goto err_class;
	}

	for (i = 0; i < WDOG_ID_MAX; i++) {

		wdog_dev->cdev[i].ops = &wdog_dev_fops;
		wdog_dev->cdev[i].owner = THIS_MODULE;

		cdev_init(&(wdog_dev->cdev[i]), &wdog_dev_fops);

		ret = cdev_add(&(wdog_dev->cdev[i]),
			 MKDEV(wdog_dev->wdog_dev_major, i), 1);
		if (ret) {
			pr_err("wdog cdev add failed: %d %d\n", i, ret);
			goto err_cdev;
		}

		if ((device_create(wdog_dev->wdog_class,
				   NULL,
				   MKDEV(wdog_dev->wdog_dev_major,
					 i),
				   NULL,
				   "%s%d", wdog_dev_name, i)) == NULL) {
			pr_err("%s: Cannot create the device"
			       , __func__);
			goto err_device;
		}
	}

	return 0;

err_device:
	cdev_del(&(wdog_dev->cdev[i]));

err_cdev:
	for (ctr = 0 ; ctr < i; ctr++) {
		device_destroy(wdog_dev->wdog_class,
			       MKDEV(wdog_dev->wdog_dev_major, ctr));
		cdev_del(&(wdog_dev->cdev[ctr]));
	}
	class_destroy(wdog_dev->wdog_class);

err_class:
	unregister_chrdev_region(wdog_dev->wdog_dev_number, WDOG_ID_MAX);

err:
	return -1;
}

static int destroy_wdog_cdevs(struct wdog_dev *wdog_dev)
{
	int i;

	for (i = 0; i < WDOG_ID_MAX; i++) {
		device_destroy(wdog_dev->wdog_class,
			       MKDEV(wdog_dev->wdog_dev_major, i));
		cdev_del(&(wdog_dev->cdev[i]));
	}

	class_destroy(wdog_dev->wdog_class);
	unregister_chrdev_region(wdog_dev->wdog_dev_number, WDOG_ID_MAX);

	return 0;
}

int wdog_init(void)
{
	int rc = 0;
	int i, j;
	struct wdog_dev *wdog_dev;
	struct device_node *dn_wdog;

	dn_wdog = of_find_node_by_name(NULL, "la9310_wdog");
	if (!dn_wdog) {
		pr_err("la9310_wdog: Node missing in DTB, skip wdog config\n");
		return rc;
	}

	wdog_dev = kmalloc(sizeof(struct wdog_dev), GFP_KERNEL);
	if (wdog_dev == NULL)
		return -ENOMEM;

	wdog_gdev = wdog_dev;

	for (i = 0; i < WDOG_ID_MAX; i++) {
		wdog_dev->wdog_pd[i].gpio =
			of_get_named_gpio(dn_wdog, "la9310-reset-gpio", i);
		if (!gpio_is_valid(wdog_dev->wdog_pd[i].gpio)) {
			pr_warn("la9310-reset-gpio %d not found\n", i);
			/*RFNM boards have different GPIO name */
			wdog_dev->wdog_pd[i].gpio =	
				of_get_named_gpio(dn_wdog, "la9310-trst-gpios", i);
			if (!gpio_is_valid(wdog_dev->wdog_pd[i].gpio)) {
				pr_err("rfnm: la9310-trst-gpios %d not found\n", i);
				rc = -EINVAL;
				goto err;
			} else {
				/* 2nd GPIO for RFNM */
				wdog_dev->wdog_pd[i].gpio_hrst =
					of_get_named_gpio(dn_wdog, "la9310-hrst-gpios", i);
				if (!gpio_is_valid(wdog_dev->wdog_pd[i].gpio_hrst)) {
					pr_err("rfnm: la9310-hrst-gpios %d not found\n", i);
					rc = -EINVAL;
					goto err;
				}
				wdog_dev->wdog_pd[i].gpio_bootstrap =
					of_get_named_gpio(dn_wdog, "la9310-bootstrap-en-gpios", i);
				if (!gpio_is_valid(wdog_dev->wdog_pd[i].gpio_bootstrap)) {
					pr_err("rfnm: la9310-bootstarp-en-gpios %d not found\n", i);
					rc = -EINVAL;
					goto err;
				}
			}

		} else {
			wdog_dev->wdog_pd[i].gpio_hrst = 0;
			wdog_dev->wdog_pd[i].gpio_bootstrap = 0;
		}
		if (wdog_gpio_config(&wdog_dev->wdog_pd[i])) {
			rc = -EINVAL;
			goto err;
		}

		init_waitqueue_head(&(wdog_dev->wdog_pd[i].wdog_wq));
		raw_spin_lock_init(&(wdog_dev->wdog_pd[i].wdog_wq_lock));

		wdog_dev->wdog_pd[i].irq_status_flag = 0;
		wdog_dev->wdog_pd[i].wdog_id = i;
		wdog_dev->wdog_pd[i].wdog_modem_status = WDOG_MODEM_NOT_READY;
		wdog_dev->wdog_pd[i].domain_nr = PCI_DOMAIN_NR_INVALID;
	}

	rc = create_wdog_cdevs(wdog_dev);
	if (rc < 0) {
		pr_err("la9310_wdog: Failed to create chardevs\n");
		goto err;
	}

	return rc;
err:
	for (j = 0; j < i; j++) {
		gpio_free(wdog_dev->wdog_pd[j].gpio);
		if (wdog_dev->wdog_pd[j].gpio_hrst)
			gpio_free(wdog_dev->wdog_pd[j].gpio_hrst);
		if (wdog_dev->wdog_pd[j].gpio_bootstrap)
			gpio_free(wdog_dev->wdog_pd[j].gpio_bootstrap);
	}
	kfree(wdog_dev);
	wdog_gdev = NULL;
	return rc;
}

int wdog_exit(void)
{
	int i;
	struct wdog_dev *wdog_dev = wdog_gdev;
	struct device_node *dn_wdog;

	dn_wdog = of_find_node_by_name(NULL, "la9310_wdog");
	if (!dn_wdog) {
		pr_err("la9310_wdog: Node missing in DTB, skip wdog config\n");
		return 0;
	}

	destroy_wdog_cdevs(wdog_dev);

	for (i = 0; i < WDOG_ID_MAX; i++) {
		gpio_free(wdog_dev->wdog_pd[i].gpio);
		if (wdog_dev->wdog_pd[i].gpio_hrst)
			gpio_free(wdog_dev->wdog_pd[i].gpio_hrst);
		if (wdog_dev->wdog_pd[i].gpio_bootstrap)
			gpio_free(wdog_dev->wdog_pd[i].gpio_bootstrap);
		if (wdog_dev->wdog_pd[i].irq_status_flag != 0)
			free_irq(wdog_dev->wdog_pd[i].irq,
				 &(wdog_dev->wdog_pd[i]));
	}

	kfree(wdog_dev);

	return 0;
}
