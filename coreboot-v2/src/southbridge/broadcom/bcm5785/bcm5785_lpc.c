/*
 * Copyright  2005 AMD
 *  by yinghai.lu@amd.com
 */

#include <console/console.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pnp.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include <pc80/mc146818rtc.h>
#include <pc80/isa-dma.h>
#include <bitops.h>
#include <arch/io.h>
#include "bcm5785.h"

static void lpc_init(device_t dev)
{

	/* Initialize the real time clock */
	rtc_init(0);

	/* Initialize isa dma */
	isa_dma_init();

}

static void bcm5785_lpc_read_resources(device_t dev)
{
	struct resource *res;
	unsigned long index;

	/* Get the normal pci resources of this device */
	pci_dev_read_resources(dev); 
	
        /* Add an extra subtractive resource for both memory and I/O */
        res = new_resource(dev, IOINDEX_SUBTRACTIVE(0, 0));
        res->flags = IORESOURCE_IO | IORESOURCE_SUBTRACTIVE | IORESOURCE_ASSIGNED;
        
        res = new_resource(dev, IOINDEX_SUBTRACTIVE(1, 0));
        res->flags = IORESOURCE_MEM | IORESOURCE_SUBTRACTIVE | IORESOURCE_ASSIGNED;

}

/**     
 * @brief Enable resources for children devices
 *      
 * @param dev the device whos children's resources are to be enabled
 *      
 * This function is call by the global enable_resources() indirectly via the
 * device_operation::enable_resources() method of devices.
 *      
 * Indirect mutual recursion:
 *      enable_childrens_resources() -> enable_resources()
 *      enable_resources() -> device_operation::enable_resources()
 *      device_operation::enable_resources() -> enable_children_resources()
 */     
static void bcm5785_lpc_enable_childrens_resources(device_t dev)
{       
        unsigned link; 
	uint32_t reg;
	int i;
	int var_num = 0;
	
	reg = pci_read_config8(dev, 0x44);

        for (link = 0; link < dev->links; link++) {
                device_t child;
                for (child = dev->link[link].children; child; child = child->sibling) {
                        enable_resources(child);
			if(child->have_resources && (child->path.type == DEVICE_PATH_PNP)) {
				for(i=0;i<child->resources;i++) {
					struct resource *res;
			                unsigned long base, end; // don't need long long
					res = &child->resource[i];
					if(!(res->flags & IORESOURCE_IO)) continue;
		        	        base = res->base;
                			end = resource_end(res);
					printk_debug("bcm5785lpc decode:%s, base=0x%08x, end=0x%08x\r\n",dev_path(child),base, end);
					switch(base) {
					case 0x60: //KBC
					case 0x64:
						reg |= (1<<29);
					case 0x3f8: // COM1
						reg |= (1<<6); 	break;
					case 0x2f8: // COM2
						reg |= (1<<7);  break; 
					case 0x378: // Parallal 1
						reg |= (1<<0); break;
					case 0x3f0: // FD0 
						reg |= (1<<26); break;
					case 0x220:  // Aduio 0
						reg |= (1<<14);	break;
					case 0x300:  // Midi 0
						reg |= (1<<18);	break;
					}
				}
			}
                }
        }
	pci_write_config32(dev, 0x44, reg);
	

}

static void bcm5785_lpc_enable_resources(device_t dev)
{
        pci_dev_enable_resources(dev);
        bcm5785_lpc_enable_childrens_resources(dev);
}

static void lpci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
        pci_write_config32(dev, 0x40,
                ((device & 0xffff) << 16) | (vendor & 0xffff));
}

static struct pci_operations lops_pci = {
        .set_subsystem = lpci_set_subsystem,
};

static struct device_operations lpc_ops  = {
	.read_resources   = bcm5785_lpc_read_resources,
	.set_resources    = pci_dev_set_resources,
	.enable_resources = bcm5785_lpc_enable_resources,
	.init             = lpc_init,
	.scan_bus         = scan_static_bus,
//	.enable           = bcm5785_enable,
	.ops_pci          = &lops_pci,
};
static const struct pci_driver lpc_driver __pci_driver = {
	.ops    = &lpc_ops,
	.vendor = PCI_VENDOR_ID_SERVERWORKS,
	.device = PCI_DEVICE_ID_SERVERWORKS_BCM5785_LPC,
};
