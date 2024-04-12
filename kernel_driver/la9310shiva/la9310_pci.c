/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2017-2024 NXP
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/version.h>

#include "la9310_pci.h"
#include "la9310_vspa.h"
#include "la9310_base.h"
#include "la9310_wdog_ioctl.h"

static const char *driver_name = "la9310-shiva";
int scratch_buf_size;
uint64_t scratch_buf_phys_addr;

int dac_mask = 0x1;
EXPORT_SYMBOL(dac_mask);
int adc_mask = 0x4;
EXPORT_SYMBOL(adc_mask);

#ifdef RFNM
int adc_rate_mask;
int dac_rate_mask;
#else
/*1 indicates the corresponding DCS at half sampling rate else full */
int adc_rate_mask = 0x4;
int dac_rate_mask = 0x1;
#endif
EXPORT_SYMBOL(adc_rate_mask);
EXPORT_SYMBOL(dac_rate_mask);

uint64_t iq_mem_addr = 0x96400000;
EXPORT_SYMBOL_GPL(iq_mem_addr);
int iq_mem_size = (1024 * 1024 * 20);
EXPORT_SYMBOL_GPL(iq_mem_size);
char firmware_name[FIRMWARE_NAME_SIZE] = FIRMWARE_RTOS;
EXPORT_SYMBOL_GPL(firmware_name);
char vspa_fw_name[FIRMWARE_NAME_SIZE] = VSPA_FW_NAME;
EXPORT_SYMBOL(vspa_fw_name);

LIST_HEAD(pcidev_list);
static int la9310_dev_id_g;
char la9310_dev_name[20];
EXPORT_SYMBOL(la9310_dev_name);

static struct class *la9310_class;

static void la9310_pcidev_remove(struct pci_dev *pdev);

static inline void __hexdump(unsigned long start, unsigned long end,
		unsigned long p, size_t sz, const unsigned char *c)
{
	while (start < end) {
		unsigned int pos = 0;
		char buf[64];
		int nl = 0;

		pos += sprintf(buf + pos, "%08lx: ", start);
		do {
			if ((start < p) || (start >= (p + sz)))
				pos += sprintf(buf + pos, "..");
			else
				pos += sprintf(buf + pos, "%02x", *(c++));
			if (!(++start & 15)) {
				buf[pos++] = '\n';
				nl = 1;
			} else {
				nl = 0;
			if (!(start & 1))
				buf[pos++] = ' ';
			if (!(start & 3))
				buf[pos++] = ' ';
			}
		} while (start & 15);
		if (!nl)
			buf[pos++] = '\n';
		buf[pos] = '\0';
		pr_info("%s", buf);
	}
}

void la9310_hexdump(const void *ptr, size_t sz)
{
	unsigned long p = (unsigned long)ptr;
	unsigned long start = p & ~(unsigned long)15;
	unsigned long end = (p + sz + 15) & ~(unsigned long)15;
	const unsigned char *c = ptr;

	__hexdump(start, end, p, sz, c);
}
EXPORT_SYMBOL_GPL(la9310_hexdump);

struct la9310_global g_la9310_global[MAX_MODEM_INSTANCES];

static int
get_la9310_dev_id_pcidevname(struct device *dev)
{
	int i;

	for (i = 0; i < MAX_MODEM_INSTANCES; i++) {
		if (!strcmp(dev_name(dev), g_la9310_global[i].dev_name)) {
			pr_info("device matched %s at Id %d\n",
				dev_name(dev), i);
			return i;
		}
	}
	if (la9310_dev_id_g >= MAX_MODEM_INSTANCES)
		return -1;
	return la9310_dev_id_g++;
}

struct la9310_dev *get_la9310_dev_byname(const char *name)
{
	struct list_head *ptr;
	struct la9310_dev *dev = NULL;

	list_for_each(ptr, &pcidev_list) {
		dev = list_entry(ptr, struct la9310_dev, list);
		if (!strcmp(dev->name, name))
			return dev;
	}

	return NULL;
}
EXPORT_SYMBOL_GPL(get_la9310_dev_byname);

ssize_t la9310_device_dump(struct la9310_dev *dev, char *buf)
{
	int i = 0;
	struct vspa_device *vspadev = (struct vspa_device *)dev->vspa_priv;
	modinfo_t mi;

	la9310_modinfo_get(dev, &mi);
	sprintf(&buf[strlen(buf)],
			" Board Name:%s, Modem ID=%d, Name:%s PCI-ID:%x, PCI_ADDR:%s\n",
			mi.board_name, dev->id, dev->name, dev->pdev->device,
			g_la9310_global[dev->id].dev_name);

	sprintf(&buf[strlen(buf)], " bin: (%s)\n", firmware_name);
	sprintf(&buf[strlen(buf)], " VSPA: (%s)\n", vspadev->eld_filename);

	sprintf(&buf[strlen(buf)], " HIF Start: addr:0x%llx len:0x%llx\n",
			mi.hif.host_phy_addr, (u64)mi.hif.size);
	sprintf(&buf[strlen(buf)], " (CCSR) BAR:%d        addr:0x%llx len:0x%llx\n",
			i, mi.ccsr.host_phy_addr, (u64)mi.ccsr.size);
	sprintf(&buf[strlen(buf)], " (TCML) BAR:%d        addr:0x%llx len:0x%llx\n",
			i, mi.tcml.host_phy_addr, (u64)mi.tcml.size);
	sprintf(&buf[strlen(buf)], " (TCMU) BAR:%d        addr:0x%llx len:0x%llx\n",
			i, mi.tcmu.host_phy_addr, (u64)mi.tcmu.size);
	sprintf(&buf[strlen(buf)], " VSPA OVERLAY BAR:%d  addr:0x%llx len:0x%llx\n",
			i, mi.ov.host_phy_addr, (u64)mi.ov.size);
	sprintf(&buf[strlen(buf)], " VSPA BAR:%d          addr:0x%llx len:0x%llx\n",
			i, mi.vspa.host_phy_addr, (u64)mi.vspa.size);
	sprintf(&buf[strlen(buf)], " FW BAR:%d            addr:0x%llx len:0x%llx\n",
			i, mi.fw.host_phy_addr, (u64)mi.fw.size);
	sprintf(&buf[strlen(buf)], " DBG LOG BAR:%d       addr:0x%llx len:0x%llx\n",
			i, mi.dbg.host_phy_addr, (u64)mi.dbg.size);
	sprintf(&buf[strlen(buf)], " IQ SAMPLES BAR:%d    addr:0x%llx len:0x%llx\n",
			i, mi.iqr.host_phy_addr, (u64)mi.iqr.size);
	sprintf(&buf[strlen(buf)], " NLM OPS BAR:%d       addr:0x%llx len:0x%llx\n",
			i, mi.nlmops.host_phy_addr, (u64)mi.nlmops.size);
	sprintf(&buf[strlen(buf)], " STD FW BAR:%d        addr:0x%llx len:0x%llx\n",
			i, mi.stdfw.host_phy_addr, (u64)mi.stdfw.size);
	sprintf(&buf[strlen(buf)], "\n Scratch:  Phy      addr:0x%llx Size:0x%llx (%lldMB)\n",
			mi.scratchbuf.host_phy_addr, (u64)mi.scratchbuf.size,
			IN_MB((u64)mi.scratchbuf.size));
	sprintf(&buf[strlen(buf)], "\n %s", "-*-*-*-");
	sprintf(&buf[strlen(buf)], "%ld\n", strlen(buf));
	return strlen(buf);
}

ssize_t la9310_show_global_status(char *buf)
{
	struct list_head *ptr;
	struct la9310_dev *dev = NULL;
	sprintf(buf, "*%s\n", LA9310_HOST_SW_VERSION);
	sprintf(&buf[strlen(buf)], " No. of LA93xx Devices Detected = %d\n",
			la9310_dev_id_g);

	list_for_each(ptr, &pcidev_list) {
		dev = list_entry(ptr, struct la9310_dev, list);
		if (g_la9310_global[dev->id].active)
			la9310_device_dump(dev, buf);
		/*  if buffer has already crossed 4K or if number of modems are more than 2
		 *  and buffer is already around 3K, better to stop.
		 */
		if (strlen(buf) > PAGE_SIZE && strlen(buf) > 3000)
			break;
	}

	sprintf(&buf[strlen(buf)], " %s", "\n");
	return strlen(buf) > PAGE_SIZE ? PAGE_SIZE : strlen(buf);
}
EXPORT_SYMBOL_GPL(la9310_show_global_status);

void la9310_dev_reset_interrupt_capability(struct la9310_dev *la9310_dev)
{
	if (LA9310_CHK_FLG(la9310_dev->flags, LA9310_FLG_PCI_MSI_EN)) {
		pci_disable_msi(la9310_dev->pdev);
		LA9310_CLR_FLG(la9310_dev->flags, LA9310_FLG_PCI_MSI_EN);
	}
}

void enable_all_msi(struct la9310_dev *la9310_dev)
{
	u32 __iomem *pcie_vaddr, *pcie_msi_control;
	u32 val;

	pcie_vaddr = (u32 *)(la9310_dev->mem_regions[LA9310_MEM_REGION_CCSR]
				.vaddr + PCIE_RHOM_DBI_BASE
					+ PCIE_MSI_BASE);

	val = ioread32(pcie_vaddr);
	dev_dbg(la9310_dev->dev, "MSI Capability: Control Reg. -> value = %x\n",
							val);
	pcie_msi_control = (u32 *)(la9310_dev->mem_regions
				   [LA9310_MEM_REGION_CCSR].vaddr +
				    PCIE_RHOM_DBI_BASE + PCIE_MSI_CONTROL);

	iowrite8(0xb6, pcie_msi_control);

	val = ioread32(pcie_vaddr);
	dev_dbg(la9310_dev->dev, "MSI Capability: Control Reg. -> value = %x\n"
		  , val);
}

/*
 * la9310_dev_set_interrupt_capability - set MSI or MSI-X if supported
 *
 * Attempt to configure interrupts using the best available
 * capabilities of the hardware and kernel.
 */
int la9310_dev_set_interrupt_capability(struct la9310_dev *la9310_dev, int mode)
{
	int ret = 0, i = 0;
	/* Check whether the device has MSIx cap */
	switch (mode) {
	case PCI_INT_MODE_MULTIPLE_MSI:
		enable_all_msi(la9310_dev);
		ret = pci_alloc_irq_vectors_affinity(la9310_dev->pdev,
				   MIN_MSI_ITR_LINES,
				   LA9310_MSI_MAX_CNT,
				   PCI_IRQ_MSI, NULL);
		if (ret < LA9310_MSI_MAX_CNT) {
			dev_err(la9310_dev->dev,
				"Cannot complete request for multiple MSI");
			goto msi_error;
		} else {
			dev_info(la9310_dev->dev,
				 "%d MSI successfully created\n", ret);
		}
		LA9310_SET_FLG(la9310_dev->flags, LA9310_FLG_PCI_MSI_EN);
		for (i = 0; i < LA9310_MSI_MAX_CNT; i++) {
			la9310_dev->irq[i].msi_val = i;
			la9310_dev->irq[i].irq_val =
				pci_irq_vector(la9310_dev->pdev, i);
			la9310_dev->irq[i].free = LA931XA_MSI_IRQ_FREE;
		}
		la9310_dev->irq_count = LA9310_MSI_MAX_CNT;
		break;

	case PCI_INT_MODE_MSIX:
		/* TBD:XXX: LA9310 will support 8 MSIs, thus MSIx. this code
		 * need to be changed
		 */
		dev_err(la9310_dev->dev,
			"Unable to support MSIX for LA9310\n");
		__attribute__((__fallthrough__));
		/* Fall through */
	case PCI_INT_MODE_MSI:
		if (!pci_enable_msi(la9310_dev->pdev)) {
			LA9310_SET_FLG(la9310_dev->flags, LA9310_FLG_PCI_MSI_EN);
			la9310_dev->irq[MSI_IRQ_MUX].irq_val =
				pci_irq_vector(la9310_dev->pdev, MSI_IRQ_MUX);
			la9310_dev->irq[MSI_IRQ_MUX].free = LA931XA_MSI_IRQ_FREE;
			la9310_dev->irq_count = 1;
		} else {
			dev_warn(la9310_dev->dev,
				 "Failed to init MSI, fall bk to legacy\n");
			goto msi_error;
		}
		__attribute__((__fallthrough__));
		/* Fall through */
	case PCI_INT_MODE_LEGACY:
		la9310_dev->irq[MSI_IRQ_MUX].irq_val = la9310_dev->pdev->irq;
		la9310_dev->irq[MSI_IRQ_MUX].free = LA931XA_MSI_IRQ_FREE;
		la9310_dev->irq_count = 1;
		break;

	case PCI_INT_MODE_NONE:
		break;
	}

	return 0;

msi_error:
	return -EINTR;
}

static void la9310_dev_free(struct la9310_dev *la9310_dev)
{
	list_del(&la9310_dev->list);

	pci_set_drvdata(la9310_dev->pdev, NULL);
	kfree(la9310_dev);
}

static int pcidev_tune_caps(struct pci_dev *pdev)
{
	struct pci_dev *parent;
	u16 pcaps, ecaps, ctl;
	int rc_sup, ep_sup;

	/* Find out supported and configured values for parent (root) */
	parent = pdev->bus->self;
	if (parent->bus->parent) {
		dev_info(&pdev->dev, "Parent not root\n");
		return -EINVAL;
	}

	if (!pci_is_pcie(parent) || !pci_is_pcie(pdev))
		return -EINVAL;

	pcie_capability_read_word(parent, PCI_EXP_DEVCAP, &pcaps);
	pcie_capability_read_word(pdev, PCI_EXP_DEVCAP, &ecaps);

	/* Find max payload supported by root, endpoint */
	rc_sup = pcaps & PCI_EXP_DEVCAP_PAYLOAD;
	ep_sup = ecaps & PCI_EXP_DEVCAP_PAYLOAD;
	dev_info(&pdev->dev, "max payload size    rc:%d ep:%d\n",
			128 * (1<<rc_sup), 128 * (1<<ep_sup));
	if (rc_sup > ep_sup)
		rc_sup = ep_sup;

	pcie_capability_clear_and_set_word(parent, PCI_EXP_DEVCTL,
					   PCI_EXP_DEVCTL_PAYLOAD, rc_sup << 5);

	pcie_capability_clear_and_set_word(pdev, PCI_EXP_DEVCTL,
					   PCI_EXP_DEVCTL_PAYLOAD, rc_sup << 5);

	pcie_capability_read_word(pdev, PCI_EXP_DEVCTL, &ctl);
	dev_dbg(&pdev->dev, "MAX payload size is %dB, MAX read size is %dB.\n",
		128 << ((ctl & PCI_EXP_DEVCTL_PAYLOAD) >> 5),
		128 << ((ctl & PCI_EXP_DEVCTL_READRQ) >> 12));

	return 0;
}


static struct la9310_dev *la9310_pci_priv_init(struct pci_dev *pdev)
{
	struct la9310_dev *la9310_dev = NULL;
	int i, rc = 0;

	la9310_dev = kzalloc(sizeof(struct la9310_dev), GFP_KERNEL);
	if (!la9310_dev)
		goto out;

	la9310_dev->dev = &pdev->dev;
	la9310_dev->pdev = pdev;

	i = get_la9310_dev_id_pcidevname(la9310_dev->dev);

	la9310_dev->id = i;

	sprintf(la9310_dev_name, "%s%d", LA9310_DEV_NAME_PREFIX, la9310_dev->id);
	sprintf(&la9310_dev->name[0], "%s", la9310_dev_name);

	/* Not allowed to create it */
	if (i == -1) {
		dev_err(&pdev->dev,
			"exceeding max permitted (%d) la9310 devices!\n",
			MAX_MODEM_INSTANCES);
		kfree(la9310_dev);
		return NULL;
	}
	la9310_dev->id = i;

	g_la9310_global[la9310_dev->id].active = 1;
	sprintf(g_la9310_global[la9310_dev->id].dev_name, "%s",
		dev_name(la9310_dev->dev));

	dev_info(la9310_dev->dev, "Init - %s !\n", la9310_dev->name);

	/* Get the BAR resources and remap them into the driver memory */
	for (i = 0; i < LA9310_MEM_REGION_BAR_END; i++) {
		/* Read the hardware address */
		la9310_dev->mem_regions[i].phys_addr = pci_resource_start(pdev,
									  i);
		la9310_dev->mem_regions[i].size = pci_resource_len(pdev, i);
		dev_info(la9310_dev->dev, "BAR:%d  addr:0x%llx len:0x%llx\n",
			 i, la9310_dev->mem_regions[i].phys_addr,
			 (u64)la9310_dev->mem_regions[i].size);
	}

	rc = la9310_map_mem_regions(la9310_dev);
	if (rc) {
		dev_err(la9310_dev->dev, "Failed to map mem regions, err %d\n",
			rc);
		goto out;
	}

#ifdef LA9310_FLG_PCI_8MSI_EN
	rc = la9310_dev_set_interrupt_capability(la9310_dev,
				PCI_INT_MODE_MULTIPLE_MSI);
#else
	rc = la9310_dev_set_interrupt_capability(la9310_dev,
				PCI_INT_MODE_MSI);
#endif
	if (rc < 0) {
		dev_info(la9310_dev->dev, "Cannot set the capability of device\n");
		goto out;
	}

	list_add_tail(&la9310_dev->list, &pcidev_list);

out:
	if (rc) {
		if (la9310_dev) {
			la9310_unmap_mem_regions(la9310_dev);
			kfree(la9310_dev);
		}
		la9310_dev = NULL;
	}

	return la9310_dev;
}

static int la9310_pcidev_probe(struct pci_dev *pdev,
				const struct pci_device_id *id)
{
	int rc = 0;
	struct la9310_dev *la9310_dev = NULL;

	rc = pci_enable_device(pdev);
	if (rc) {
		dev_err(&pdev->dev, "failed to enable\n");
		goto err1;
	}

	rc = pci_request_regions(pdev, driver_name);
	if (rc) {
		dev_err(&pdev->dev, "failed to request pci regions\n");
		goto err2;
	}

	pci_set_master(pdev);

	rc = dma_set_mask(&pdev->dev, DMA_BIT_MASK(64));
	if (rc) {
		rc = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (rc) {
			dev_err(&pdev->dev, "Could not set PCI Mask\n");
			goto err3;
		}
	}
	pcidev_tune_caps(pdev);

	/*Initialize la9310_dev from information obtained from pci_dev*/
	la9310_dev = la9310_pci_priv_init(pdev);
	if (!la9310_dev) {
		rc = -ENOMEM;
		dev_err(&pdev->dev, "la9310_pci_priv_init failed, err %d\n",
			rc);
		goto err4;
	}

	la9310_dev->class = la9310_class;

	rc = la9310_base_probe(la9310_dev);
	if (rc) {
		dev_err(la9310_dev->dev, "la9310_base_probe failed, err %d\n",
			rc);
		goto err5;
	}

	pci_set_drvdata(pdev, la9310_dev);

	return rc;

err5:
	la9310_dev_reset_interrupt_capability(la9310_dev);

err4:
	if (la9310_dev)
		la9310_dev_free(la9310_dev);

err3:
	pci_release_regions(pdev);

err2:
	pci_disable_device(pdev);

err1:
	return rc;
}

static void la9310_pcidev_remove(struct pci_dev *pdev)
{
	struct la9310_dev *la9310_dev = pci_get_drvdata(pdev);

	if (!la9310_dev)
		return;

	la9310_base_remove(la9310_dev);
	la9310_unmap_mem_regions(la9310_dev);
	la9310_dev_reset_interrupt_capability(la9310_dev);
	la9310_dev_free(la9310_dev);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}

static const struct pci_device_id la9310_pcidev_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_LA9310) },
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_LA9310_DISABLE_CIP)
	 },
};

static struct pci_driver la9310_pcidev_driver = {
	.name		= "NXP-LA9310-Driver",
	.id_table	= la9310_pcidev_ids,
	.probe		= la9310_pcidev_probe,
	.remove		= la9310_pcidev_remove
};

static int __init la9310_pcidev_init(void)
{
	int err = 0;

	pr_info("NXP PCIe LA9310 Driver: Init.\n");

	if (!(scratch_buf_size && scratch_buf_phys_addr)) {
		pr_err("ERR %s: Scratch buf values should be non zero\n",
		       __func__);
		err = -EINVAL;
		goto out;
	}

	if ((scratch_buf_size <= LA9310_VSPA_FW_SIZE) ||
	    (scratch_buf_size > LA9310_MAX_SCRATCH_BUF_SIZE)) {
		pr_err("ERR %s: Scratch_buf_size is not correct 0x%x (Range - 0x%x-0x%x)\n", __func__,
			scratch_buf_size, LA9310_VSPA_FW_SIZE, LA9310_MAX_SCRATCH_BUF_SIZE);
		err = -EINVAL;
		goto out;
	}

	if (!(strlen(vspa_fw_name))) {
		pr_err("ERR %s: alt_vspa_fw_name empty", __func__);
		err = -EINVAL;
		goto out;
	}

	la9310_class = class_create(THIS_MODULE, driver_name);
	if (IS_ERR(la9310_class)) {
		pr_err("%s:%d Error in creating (%s) class\n",
			__func__, __LINE__, driver_name);
		return PTR_ERR(la9310_class);
	}

	err = la9310_subdrv_mod_init();
	if (err)
		goto out;

	err = pci_register_driver(&la9310_pcidev_driver);
	if (err) {
		pr_err("%s:%d pci_register_driver() failed!\n",
			__func__, __LINE__);
	}
	la9310_init_global_sysfs();
out:
	return err;
}

static void __exit la9310_pcidev_exit(void)
{
	la9310_remove_global_sysfs ();
	pci_unregister_driver(&la9310_pcidev_driver);
	la9310_subdrv_mod_exit();
	class_destroy(la9310_class);
	pr_err("NXP PCIe LA9310 Driver: Exit\n");
}

module_init(la9310_pcidev_init);
module_exit(la9310_pcidev_exit);
module_param(scratch_buf_size, int, 0);
MODULE_PARM_DESC(scratch_buf_size, "Scratch buffer size for LA9310 Device");
module_param(scratch_buf_phys_addr, ullong, 0);
MODULE_PARM_DESC(scratch_buf_phys_addr, "Scratch buffer start physical address");
module_param(adc_mask, int, 0400);
MODULE_PARM_DESC(adc_mask, "ADC channel enable mask - bit wise (MAX 0x4)");
module_param(adc_rate_mask, int, 0400);
MODULE_PARM_DESC(adc_rate_mask,
	"ADC Frequency mask for each channel (0 for Full Duplex, 1 for Half Duplex)");
module_param(dac_mask, int, 0400);
MODULE_PARM_DESC(dac_mask, "DAC channel enable mask - bit wise (MAX 0x2)");
module_param(dac_rate_mask, int, 0400);
MODULE_PARM_DESC(dac_rate_mask,
	"DAC Frequency for each channel (0 for Full Duplex, 1 for Half Duplex");
module_param(iq_mem_addr, ullong, 0400);
MODULE_PARM_DESC(iq_mem_addr, "RFNM IQ Flood Mem Address");
module_param(iq_mem_size, int, 0400);
MODULE_PARM_DESC(iq_mem_addr, "RFNM IQ Flood Mem Size");
module_param_string(alt_vspa_fw_name, vspa_fw_name,
		sizeof(vspa_fw_name), 0400);
MODULE_PARM_DESC(alt_vspa_fw_name,
		"Alternative VSP firmware name prefix e.g apm.eld");
module_param_string(alt_firmware_name, firmware_name,
		sizeof(firmware_name), 0400);
MODULE_PARM_DESC(alt_firmware_name,
	"Alternative shiva firmware name e.g la9310.bin");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("NXP");
MODULE_DESCRIPTION("PCIe LA9310 Driver");
MODULE_VERSION(LA9310_HOST_SW_VERSION);
