/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2017-2024 NXP
 */

#include <linux/module.h>
#include <linux/moduleparam.h>

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/eventfd.h>
#include <linux/fdtable.h>

#include "la93xx_ipc_ioctl.h"
#include "la9310_base.h"
#include "la93xx_bbdev_ipc.h"

#define DEVICE_NAME_LEN	32
#define IPC_NR_DEVICES		2
#define IPC_MINOR_START	0

#define LA9310_IPC_IRQ_NUM_TXT_SIZE	64
#define SIGNAL_TO_CHANNEL_LISTENER	1

static uint32_t ipc_major;
static uint32_t ipc_minor;
static dev_t ipc_devnr;
static uint32_t ipc_nr_dev;

static uint8_t ipc_minor_index;
static uint8_t in_use_minor[IPC_NR_DEVICES];

struct ipc_Irq {
	int irq_num;
	int msi_value;
	int irq_index;
	char *irq_num_txt;
};

struct ipc_chan {
	struct eventfd_ctx *evt_fd_ctxt;
	struct ipc_Irq ipc_irq;
};

struct ipc_dev {
	char name[DEVICE_NAME_LEN];

	/* LA9310 Device */
	struct la9310_dev *la9310_dev;

	/* Linux character device for IPC */
	struct cdev cdev;

	/* IPC Device Number */
	dev_t ipc_devnr;

	/* IPC device minor number*/
	uint8_t minor;

	/* IPC channel and IRQ mapping */
	struct ipc_chan asyn_chans[IPC_MAX_CHANNEL_COUNT];

	sys_map_t sys_map;
};

char ipc_dev_name[DEVICE_NAME_LEN];

static int
ipc_open(struct inode *inode, struct file *filp)
{
	struct ipc_dev *dev = container_of(inode->i_cdev,
					   struct ipc_dev, cdev);

	filp->private_data = dev;

	return 0;
}

static int
ipc_release(struct inode *inode, struct file *filp)
{
	filp->private_data = NULL;

	return 0;
}

int la9310_dev_get_ipc_msi(int ipc_ch_num)
{
	if (ipc_ch_num == 0)
		return MSI_IRQ_IPC_1;
	else if (ipc_ch_num == 1)
		return MSI_IRQ_IPC_2;
	else
		return -1;
}

static irqreturn_t ipc_irq_handler(int irq, void *data)
{
	struct ipc_chan *ipc_chan = (struct ipc_chan *)data;

	/* Send signal to IPC Channel Listner */
	if (ipc_chan)
		eventfd_signal(ipc_chan->evt_fd_ctxt,
				SIGNAL_TO_CHANNEL_LISTENER);

	return IRQ_HANDLED;
}

int register_ipc_channel_irq(struct la9310_dev *la9310_dev, int ipc_ch_num,
				uint32_t fd)
{
	int ret = 0, irq_num = 0, index = 0;
	int ipc_interrupt_flags = IRQF_TRIGGER_RISING;
	struct task_struct *userspace_task = NULL;
	struct file *efd_file = NULL;
	struct ipc_chan *ipc_chan = NULL;

	if (ipc_ch_num >= IPC_MAX_CHANNEL_COUNT) {
		dev_err(la9310_dev->dev,
		"%s : error : Invalid IPC channel number\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	if (!la9310_dev || !la9310_dev->ipc_priv) {
		dev_err(la9310_dev->dev,
		"%s : error : la9310_dev is NULL\n", __func__);
		ret = -EINVAL;
		goto err;
	} else {
		ipc_chan = &((struct ipc_dev *)
			(la9310_dev->ipc_priv))->asyn_chans[ipc_ch_num];
	}

	/* Register Async Notifications */
	index = la9310_dev_get_ipc_msi(ipc_ch_num);
	if (index >= 0 && index < LA9310_MSI_MAX_CNT) {
		ipc_chan->ipc_irq.irq_num = la9310_dev->irq[index].irq_val;
		ipc_chan->ipc_irq.msi_value = la9310_dev->irq[index].msi_val;
		ipc_chan->ipc_irq.irq_index = index;
	} else {
		/* Return error : No free IRQ line */
		ret = -EBUSY;
		goto err;
	}

	/* Get current task context from which IOCTL was called */
	userspace_task = current;

	rcu_read_lock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0)
	efd_file = files_lookup_fd_rcu(userspace_task->files, fd);
#else
	efd_file = fcheck_files(userspace_task->files, fd);
#endif
	rcu_read_unlock();

	ipc_chan->evt_fd_ctxt = eventfd_ctx_fileget(efd_file);
	if (ipc_chan->evt_fd_ctxt != NULL) {
		/* IRQ request */
		irq_num = ipc_chan->ipc_irq.irq_num;
		ipc_chan->ipc_irq.irq_num_txt = kmalloc(LA9310_IPC_IRQ_NUM_TXT_SIZE, GFP_KERNEL);
		if (!ipc_chan->ipc_irq.irq_num_txt) {
			dev_err(la9310_dev->dev, "no memory\n");
			goto err;
		}
		sprintf(ipc_chan->ipc_irq.irq_num_txt, "ipc_ch_irq_%d", irq_num);
		ret = request_irq(irq_num, ipc_irq_handler,
			ipc_interrupt_flags, ipc_chan->ipc_irq.irq_num_txt, ipc_chan);
		if (ret < 0) {
			dev_err(la9310_dev->dev,
				"%s request irq err - %d\n", __func__, ret);
			goto err;
		}
	} else {
		dev_err(la9310_dev->dev,
			"%s: efd file(0x%p) or evt_fd_ctxt(0x%p) is invalid\n",
			__func__, efd_file, ipc_chan->evt_fd_ctxt);
		ret = -EINVAL;
		goto err;
	}
err:
	return ret;
}

void deregister_ipc_channel_irq(struct la9310_dev *la9310_dev, int ipc_ch_num)
{
	struct ipc_chan *ipc_chan = &((struct ipc_dev *)
			(la9310_dev->ipc_priv))->asyn_chans[ipc_ch_num];

	/* IRQ request */
	kfree(ipc_chan->ipc_irq.irq_num_txt);
	free_irq(ipc_chan->ipc_irq.irq_num, ipc_chan);
	ipc_chan->ipc_irq.irq_index = 0;
}

static long ipc_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	struct ipc_dev *ipc_dev = (struct ipc_dev *)filp->private_data;
	struct la9310_dev *la9310_dev = ipc_dev->la9310_dev;
	ipc_eventfd_t ipc_channel = {0};
	sys_map_t *sys_map_user = (sys_map_t *)arg;
	sys_map_t sys_map;
	struct la9310_mem_region_info *nlm_ops_region;
	int offset;
	int ret = 0, channel_id;

	switch (cmd) {
	case  IOCTL_LA93XX_IPC_GET_SYS_MAP:
		ret = copy_from_user(&sys_map,
					(sys_map_t *)arg, sizeof(sys_map_t));
		if (ret != 0)
			return -EFAULT;

		if (la9310_dev) {
			if (LA9310_USER_HUGE_PAGE_PHYS_ADDR + sys_map.hugepg_start.size >= PCI_OUTBOUND_WINDOW_BASE_ADDR + MAX_OUTBOUND_WINDOW) {
				printk(KERN_INFO "IPC Outbound window size exceeds available outbound window size\n");
				printk(KERN_INFO "Hugepage base addr:0x%x Huge size:0x%x Outbound base addr:0x%x Max window size:0x%x\n",
					LA9310_USER_HUGE_PAGE_PHYS_ADDR,
					sys_map.hugepg_start.size,
					PCI_OUTBOUND_WINDOW_BASE_ADDR,
					MAX_OUTBOUND_WINDOW);
				return -ENOMEM;
			}
			la9310_create_ipc_hugepage_outbound(la9310_dev,
				sys_map.hugepg_start.host_phys,
				sys_map.hugepg_start.size);

			sys_map.mhif_start.host_phys =
				la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].phys_addr +
				LA9310_EP_HIF_OFFSET;
			sys_map.mhif_start.size = LA9310_EP_HIF_SIZE;
			printk(KERN_INFO "la9310_dev->hif->ipc_regs.ipc_mdata_size: %d\n",
				la9310_dev->hif->ipc_regs.ipc_mdata_size);

			sys_map.hugepg_start.modem_phys =
				(uint32_t)LA9310_USER_HUGE_PAGE_PHYS_ADDR;
			sys_map.hugepg_start.size =
				sys_map_user->hugepg_start.size;

			sys_map.tcml_start.host_phys =
				la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].phys_addr;
			sys_map.tcml_start.size =
				la9310_dev->mem_regions[LA9310_MEM_REGION_TCML].size;

			sys_map.modem_ccsrbar.host_phys =
				la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR].phys_addr;
			sys_map.modem_ccsrbar.size =
				la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR].size;

			nlm_ops_region = la9310_get_dma_region(la9310_dev,
							       LA9310_MEM_REGION_NLM_OPS);
			offset = nlm_ops_region->phys_addr -
				 la9310_dev->dma_info.ep_pcie_addr;
			sys_map.nlm_ops.host_phys =
				la9310_dev->dma_info.host_buf.phys_addr + offset;
			sys_map.nlm_ops.modem_phys = nlm_ops_region->phys_addr;
			sys_map.nlm_ops.size = nlm_ops_region->size;

			memcpy(&ipc_dev->sys_map, &sys_map, sizeof(sys_map_t));
			ret = copy_to_user(sys_map_user, &sys_map,
						sizeof(sys_map_t));
			if (ret != 0)
				return -EFAULT;

			/* All is done. Set Host Library Ready now. */
			la9310_set_host_ready(la9310_dev, LA9310_HIF_STATUS_IPC_LIB_READY);
		} else {
			ret = -ENODEV;
		}

	break;

	/* Register IRQ for PCI channel passed and
	 *  return MSI Number to calling task
	 */
	case  IOCTL_LA93XX_IPC_CHANNEL_REGISTER:
		ret = copy_from_user(&ipc_channel, (ipc_eventfd_t *)arg,
				sizeof(ipc_eventfd_t));
		if (ret != 0)
			return -EFAULT;

		ret = register_ipc_channel_irq(la9310_dev,
					ipc_channel.ipc_channel_num,
					ipc_channel.efd);
		if (ret < 0)
			return -EFAULT;

		/* Return msi_value to caller (mapped to IRQ) */
		ipc_channel.msi_value =
			ipc_dev->asyn_chans
			[ipc_channel.ipc_channel_num].ipc_irq.msi_value;
		ret = copy_to_user((void *)arg, (void *)&ipc_channel,
					sizeof(ipc_eventfd_t));
		if (ret != 0)
			return -EFAULT;
	break;

	/* De-register PCI channel passed and free up associated IRQ */
	case IOCTL_LA93XX_IPC_CHANNEL_DEREGISTER:
		ret = copy_from_user(&ipc_channel,
			(ipc_eventfd_t *)arg, sizeof(ipc_eventfd_t));
		if (ret != 0)
			return -EFAULT;

		deregister_ipc_channel_irq(la9310_dev,
			ipc_channel.ipc_channel_num);
	break;

	case IOCTL_LA93XX_IPC_CHANNEL_RAISE_INTERRUPT:
		/* Raise requested MSI for channel.*/
		ret = copy_from_user(&channel_id, (int *)arg, sizeof(int));
		if (ret != 0)
			return -EFAULT;

		/* We cannot support more than 32 channels (i.e. ibs values
		 * limit in calling raise_msg_interrupt)
		 */
		if (channel_id >= 32)
			return -EINVAL;

		raise_msg_interrupt(la9310_dev, LA9310_MSG_UNIT3, channel_id);

	break;

	default:
		ret = -ENOTTY;
	}

	return ret;
}
static const struct file_operations ipc_fops = {
	.owner = THIS_MODULE,
	.open =	ipc_open,
	.release = ipc_release,
	.unlocked_ioctl = ipc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ipc_ioctl,
#endif
};

static int
ipc_create_cdev(struct ipc_dev *ipc_dev)
{
	int ret = 0;

	cdev_init(&ipc_dev->cdev, &ipc_fops);
	ipc_dev->cdev.owner = THIS_MODULE;
	ipc_dev->cdev.ops = &ipc_fops;
	ret = cdev_add(&ipc_dev->cdev, ipc_dev->ipc_devnr, 1);
	if (ret)
		printk(KERN_CRIT "Error %d adding %s\n", ret, ipc_dev->name);

	return ret;
}

int
la9310_ipc_probe(struct la9310_dev *la9310_dev, int virq_count,
	      struct virq_evt_map *virq_map)
{
	int ret = 0, retries  = LA9310_IPC_INIT_WAIT_RETRIES;
	struct la9310_hif *hif;
	struct ipc_dev *ipc_dev;
	struct device *ipc_class_dev = NULL;
	uint32_t i;

	dev_info(la9310_dev->dev, "Inside %s function K_hif=%lx\n", __func__,
			sizeof(struct la9310_hif));

	for (i = 0; i < IPC_NR_DEVICES; i++) {
		if (in_use_minor[i] == 0) {
			ipc_minor_index = i;
			break;
		}
	}

	if (i == IPC_NR_DEVICES) {
		printk(KERN_ERR "No minor no. free to create ipc dev\n");
		return -ENODEV;
	}

	ipc_dev = kmalloc(sizeof(struct ipc_dev), GFP_KERNEL);
	if (!ipc_dev) {
		printk(KERN_CRIT "Memory allocation failure for ipc_dev\n");
		return -ENOMEM;
	}

	ipc_dev->ipc_devnr = MKDEV(ipc_major, ipc_minor_index);
	sprintf(ipc_dev->name, "%s", ipc_dev_name);

	ipc_class_dev = device_create(la9310_dev->class, NULL,
				ipc_dev->ipc_devnr,
				NULL, ipc_dev->name);
	if (IS_ERR(ipc_class_dev))
		goto fail;

	ipc_dev->la9310_dev = la9310_dev;

	/* Wait for Modem to ready IPC metadata */
	hif = la9310_dev->hif;
	while (!CHK_HIF_MOD_RDY(hif, LA9310_HIF_MOD_READY_IPC_LIB) &&
			retries) {
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(
				 LA9310_IPC_INIT_WAIT_TIMEOUT));
		retries--;
		dma_rmb();
	}

	if (!CHK_HIF_MOD_RDY(hif, LA9310_HIF_MOD_READY_IPC_LIB)) {
		dev_err(la9310_dev->dev, "IPC modem ready not set, Aborting!\n");
		ret = -EBUSY;
		goto fail;
	}
	dev_info(la9310_dev->dev, "IPC modem is ready!\n");

	ret = ipc_create_cdev(ipc_dev);
	if (ret)
		goto fail;

	la9310_dev->ipc_priv = ipc_dev;

	in_use_minor[ipc_minor_index] = 1;
	ipc_dev->minor = ipc_minor_index;

	dev_info(la9310_dev->dev, "Exiting function %s\n", __func__);
	return ret;

fail:
	if (ipc_class_dev) {
		if (IS_ERR(ipc_class_dev))
			ret = PTR_ERR(ipc_class_dev);
		else
			device_destroy(la9310_dev->class, ipc_dev->ipc_devnr);
	}

	if (ipc_dev)
		kfree(ipc_dev);

	return ret;
}

int
la9310_ipc_remove(struct la9310_dev *la9310_dev)
{
	struct ipc_dev *ipc_dev;

	ipc_dev = la9310_dev->ipc_priv;
	if (ipc_dev) {
		cdev_del(&ipc_dev->cdev);
		device_destroy(la9310_dev->class, ipc_dev->ipc_devnr);
		in_use_minor[ipc_dev->minor] = 0;
		kfree(ipc_dev);
		la9310_dev->ipc_priv = NULL;
	}

	return 0;
}

int
la9310_ipc_init(void)
{
	int ret;

	ipc_devnr = 0;
	ipc_minor_index = IPC_MINOR_START;
	ipc_minor = IPC_MINOR_START;
	ipc_nr_dev = IPC_NR_DEVICES;

	sprintf(ipc_dev_name, "%s%s",
		LA9310_DEV_NAME_PREFIX, LA9310_IPC_DEVNAME_PREFIX);

	ret = alloc_chrdev_region(&ipc_devnr, ipc_minor,
			ipc_nr_dev, ipc_dev_name);
	if (ret < 0) {
		printk(KERN_CRIT "la9310_ipc: Failed in getting major number\n");
		return ret;
	}

	ipc_major = MAJOR(ipc_devnr);

	printk(KERN_INFO "LA9310 IPC driver: major_nr %d, minor %d\n",
			ipc_major, ipc_minor);

	return ret;
}
EXPORT_SYMBOL_GPL(la9310_ipc_init);

int
la9310_ipc_exit(void)
{
	unregister_chrdev_region(ipc_devnr, ipc_nr_dev);
	return 0;
}
EXPORT_SYMBOL_GPL(la9310_ipc_exit);
