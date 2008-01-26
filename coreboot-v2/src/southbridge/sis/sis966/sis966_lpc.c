/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2003 Linux Networx
 * Copyright (C) 2003 SuSE Linux AG
 * Copyright (C) 2004 Tyan Computer
 * Written by Yinghai Lu <yhlu@tyan.com> for Tyan Computer.
 * Copyright (C) 2006,2007 AMD
 * Written by Yinghai Lu <yinghai.lu@amd.com> for AMD.
 * Copyright (C) 2007 Silicon Integrated Systems Corp. (SiS)
 * Written by Morgan Tsai <my_tsai@sis.com> for SiS.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
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
#include <cpu/x86/lapic.h>
#include "sis966.h"
#include <pc80/keyboard.h>

#define NMI_OFF	0

struct ioapicreg {
	unsigned int reg;
	unsigned int value_low, value_high;
};

static struct ioapicreg ioapicregvalues[] = {
#define ALL		(0xff << 24)
#define NONE		(0)
#define DISABLED	(1 << 16)
#define ENABLED		(0 << 16)
#define TRIGGER_EDGE	(0 << 15)
#define TRIGGER_LEVEL	(1 << 15)
#define POLARITY_HIGH	(0 << 13)
#define POLARITY_LOW	(1 << 13)
#define PHYSICAL_DEST	(0 << 11)
#define LOGICAL_DEST	(1 << 11)
#define ExtINT		(7 << 8)
#define NMI		(4 << 8)
#define SMI		(2 << 8)
#define INT		(1 << 8)
	/* IO-APIC virtual wire mode configuration */
	/* mask, trigger, polarity, destination, delivery, vector */
	{   0, ENABLED | TRIGGER_EDGE | POLARITY_HIGH | PHYSICAL_DEST | ExtINT, NONE},
	{   1, DISABLED, NONE},
	{   2, DISABLED, NONE},
	{   3, DISABLED, NONE},
	{   4, DISABLED, NONE},
	{   5, DISABLED, NONE},
	{   6, DISABLED, NONE},
	{   7, DISABLED, NONE},
	{   8, DISABLED, NONE},
	{   9, DISABLED, NONE},
	{  10, DISABLED, NONE},
	{  11, DISABLED, NONE},
	{  12, DISABLED, NONE},
	{  13, DISABLED, NONE},
	{  14, DISABLED, NONE},
	{  15, DISABLED, NONE},
	{  16, DISABLED, NONE},
	{  17, DISABLED, NONE},
	{  18, DISABLED, NONE},
	{  19, DISABLED, NONE},
	{  20, DISABLED, NONE},
	{  21, DISABLED, NONE},
	{  22, DISABLED, NONE},
	{  23, DISABLED, NONE},
	/* Be careful and don't write past the end... */
};

static void setup_ioapic(unsigned long ioapic_base)
{
	int i;
	unsigned long value_low, value_high;
//	unsigned long ioapic_base = 0xfec00000;
	volatile unsigned long *l;
	struct ioapicreg *a = ioapicregvalues;

	ioapicregvalues[0].value_high = lapicid()<<(56-32);

	l = (unsigned long *) ioapic_base;

	for (i = 0; i < sizeof(ioapicregvalues) / sizeof(ioapicregvalues[0]);
	     i++, a++) {
		l[0] = (a->reg * 2) + 0x10;
		l[4] = a->value_low;
		value_low = l[4];
		l[0] = (a->reg *2) + 0x11;
		l[4] = a->value_high;
		value_high = l[4];
		if ((i==0) && (value_low == 0xffffffff)) {
			printk_warning("IO APIC not responding.\n");
			return;
		}
		printk_spew("for IRQ, reg 0x%08x value 0x%08x 0x%08x\n",
			    a->reg, a->value_low, a->value_high);
	}
}

// 0x7a or e3
#define PREVIOUS_POWER_STATE	0x7A

#define MAINBOARD_POWER_OFF	0
#define MAINBOARD_POWER_ON	1
#define SLOW_CPU_OFF	0
#define SLOW_CPU__ON	1

#ifndef MAINBOARD_POWER_ON_AFTER_POWER_FAIL
#define MAINBOARD_POWER_ON_AFTER_POWER_FAIL	MAINBOARD_POWER_ON
#endif

static void lpc_common_init(device_t dev)
{
	uint8_t byte;
	uint32_t dword;

	/* IO APIC initialization */
	byte = pci_read_config8(dev, 0x74);
	byte |= (1<<0); // enable APIC
	pci_write_config8(dev, 0x74, byte);
	dword = pci_read_config32(dev, PCI_BASE_ADDRESS_1); // 0x14

	setup_ioapic(dword);

}

static void lpc_slave_init(device_t dev)
{
	lpc_common_init(dev);
}


static void lpc_usb_legacy_init(device_t dev)
{
    uint16_t acpi_base;

    acpi_base = (pci_read_config8(dev,0x75) << 8);

    outb(inb(acpi_base + 0xbb) |0x80, acpi_base + 0xbb);
    outb(inb(acpi_base + 0xba) |0x80, acpi_base + 0xba);
}

static void lpc_init(device_t dev)
{
	 uint8_t byte;
	 uint8_t byte_old;
	 int on;
	 int nmi_option;

        printk_debug("LPC_INIT -------->\n");
        init_pc_keyboard(0x60, 0x64, 0);

        lpc_usb_legacy_init(dev);
	 lpc_common_init(dev);

	/* power after power fail */


	on = MAINBOARD_POWER_ON_AFTER_POWER_FAIL;
	get_option(&on, "power_on_after_fail");
	byte = pci_read_config8(dev, PREVIOUS_POWER_STATE);
	byte &= ~0x40;
	if (!on) {
		byte |= 0x40;
	}
	pci_write_config8(dev, PREVIOUS_POWER_STATE, byte);
	printk_info("set power %s after power fail\n", on?"on":"off");

	/* Throttle the CPU speed down for testing */
	on = SLOW_CPU_OFF;
	get_option(&on, "slow_cpu");
	if(on) {
		uint16_t pm10_bar;
		uint32_t dword;
		pm10_bar = (pci_read_config16(dev, 0x60)&0xff00);
		outl(((on<<1)+0x10)  ,(pm10_bar + 0x10));
		dword = inl(pm10_bar + 0x10);
		on = 8-on;
		printk_debug("Throttling CPU %2d.%1.1d percent.\n",
				(on*12)+(on>>1),(on&1)*5);
	}

        /* Enable Error reporting */
        /* Set up sync flood detected */
        byte = pci_read_config8(dev, 0x47);
        byte |= (1 << 1);
        pci_write_config8(dev, 0x47, byte);

        /* Set up NMI on errors */
        byte = inb(0x70); // RTC70
        byte_old = byte;
        nmi_option = NMI_OFF;
        get_option(&nmi_option, "nmi");
        if (nmi_option) {
                byte &= ~(1 << 7); /* set NMI */
        } else {
                byte |= ( 1 << 7); // Can not mask NMI from PCI-E and NMI_NOW
        }
        if( byte != byte_old) {
                outb(0x70, byte);
        }

        /* Initialize the real time clock */
        rtc_init(0);

        /* Initialize isa dma */
        isa_dma_init();

        printk_debug("LPC_INIT <--------\n");
}

static void sis966_lpc_read_resources(device_t dev)
{
	struct resource *res;

	/* Get the normal pci resources of this device */
	pci_dev_read_resources(dev); // We got one for APIC, or one more for TRAP

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
static void sis966_lpc_enable_childrens_resources(device_t dev)
{
	unsigned link;
	uint32_t reg, reg_var[4];
	int i;
	int var_num = 0;

	reg = pci_read_config32(dev, 0xa0);

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
					printk_debug("sis966 lpc decode:%s, base=0x%08x, end=0x%08x\n",dev_path(child),base, end);
					switch(base) {
					case 0x3f8: // COM1
						reg |= (1<<0);	break;
					case 0x2f8: // COM2
						reg |= (1<<1);  break;
					case 0x378: // Parallal 1
						reg |= (1<<24); break;
					case 0x3f0: // FD0
						reg |= (1<<20); break;
					case 0x220:  // Aduio 0
						reg |= (1<<8);	break;
					case 0x300:  // Midi 0
						reg |= (1<<12);	break;
					}
					if( (base == 0x290) || (base >= 0x400)) {
						if(var_num>=4) continue; // only 4 var ; compact them ?
						reg |= (1<<(28+var_num));
						reg_var[var_num++] = (base & 0xffff)|((end & 0xffff)<<16);
					}
				}
			}
		}
	}
	pci_write_config32(dev, 0xa0, reg);
	for(i=0;i<var_num;i++) {
		pci_write_config32(dev, 0xa8 + i*4, reg_var[i]);
	}


}

static void sis966_lpc_enable_resources(device_t dev)
{
	pci_dev_enable_resources(dev);
	sis966_lpc_enable_childrens_resources(dev);
}

static void lpci_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	pci_write_config32(dev, 0x40,
		((device & 0xffff) << 16) | (vendor & 0xffff));
}

static struct pci_operations lops_pci = {
	.set_subsystem	= lpci_set_subsystem,
};

static struct device_operations lpc_ops  = {
	.read_resources	= sis966_lpc_read_resources,
	.set_resources	= pci_dev_set_resources,
	.enable_resources	= sis966_lpc_enable_resources,
	.init		= lpc_init,
	.scan_bus	= scan_static_bus,
//	.enable		= sis966_enable,
	.ops_pci	= &lops_pci,
};
static const struct pci_driver lpc_driver __pci_driver = {
	.ops	= &lpc_ops,
	.vendor	= PCI_VENDOR_ID_SIS,
	.device	= PCI_DEVICE_ID_SIS_SIS966_LPC,
};

static struct device_operations lpc_slave_ops  = {
	.read_resources	= sis966_lpc_read_resources,
	.set_resources	= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init		= lpc_slave_init,
//	.enable		= sis966_enable,
	.ops_pci	= &lops_pci,
};