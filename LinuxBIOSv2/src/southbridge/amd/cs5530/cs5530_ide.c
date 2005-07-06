#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include "cs5530.h"

static void ide_init(struct device *dev)
{
	printk_spew("cs5530_ide: %s\n", __FUNCTION__);
}

static void ide_enable(struct device *dev)
{
	printk_spew("cs5530_ide: %s\n", __FUNCTION__);
}

static struct device_operations ide_ops = {
	.read_resources   = pci_dev_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init             = ide_init,
	.enable           = ide_enable,
};

static struct pci_driver ide_driver __pci_driver = {
	.ops 	= &ide_ops,
	.vendor = PCI_VENDOR_ID_CYRIX,
	.device = PCI_DEVICE_ID_CYRIX_5530_IDE,
};
