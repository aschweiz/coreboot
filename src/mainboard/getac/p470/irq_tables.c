/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2008 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <arch/pirq_routing.h>

const struct irq_routing_table intel_irq_routing_table = {
	PIRQ_SIGNATURE,  /* u32 signature */
	PIRQ_VERSION,    /* u16 version   */
	32+16*CONFIG_IRQ_SLOT_COUNT,	 /* There can be total 18 devices on the bus */
	0x00,		 /* Where the interrupt router lies (bus) */
	(0x1f<<3)|0x0,   /* Where the interrupt router lies (dev) */
	0,		 /* IRQs devoted exclusively to PCI usage */
	0x8086,		 /* Vendor */
	0x27b0,		 /* Device */
	0,		 /* miniport */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* u8 rfu[11] */
	0xf,		 /* u8 checksum. */
	{
		/* bus,     dev|fn,   {link, bitmap}, {link, bitmap}, {link, bitmap}, {link, bitmap},  slot, rfu */
		{0x00,(0x01<<3)|0x0, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdcf8}, {0x63, 0x0dcd8}}, 0x0, 0x0}, // PCIe?
		{0x00,(0x02<<3)|0x0, {{0x60, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x00000}}, 0x0, 0x0}, // VGA
		{0x00,(0x1e<<3)|0x0, {{0x61, 0xdcf8}, {0x68, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x00000}}, 0x0, 0x0}, // PCI bridge
		{0x00,(0x1f<<3)|0x0, {{0x62, 0xdcf8}, {0x63, 0xdcd8}, {0x00, 0x0000}, {0x00, 0x00000}}, 0x0, 0x0}, // LPC
		{0x00,(0x1d<<3)|0x0, {{0x6b, 0xdcf8}, {0x63, 0xdcd8}, {0x62, 0xdcf8}, {0x60, 0x0dcf8}}, 0x0, 0x0}, // USB#1
		{0x00,(0x1b<<3)|0x0, {{0x60, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x00000}}, 0x0, 0x0}, // Audio device
		{0x00,(0x1c<<3)|0x0, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdcf8}, {0x63, 0x0dcd8}}, 0x2, 0x0}, // PCIe bridge
		{0x04,(0x00<<3)|0x0, {{0x60, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x00000}}, 0x0, 0x0}, // Firewire
		{0x04,(0x01<<3)|0x0, {{0x68, 0xdcf8}, {0x69, 0xdcf8}, {0x6a, 0xdcf8}, {0x6b, 0x0dcf8}}, 0x1, 0x0}, // PCI Bridge
		{0x04,(0x02<<3)|0x0, {{0x69, 0xdcf8}, {0x6a, 0xdcf8}, {0x6b, 0xdcf8}, {0x68, 0x0dcf8}}, 0x2, 0x0},
		{0x04,(0x03<<3)|0x0, {{0x6a, 0xdcf8}, {0x6b, 0xdcf8}, {0x68, 0xdcf8}, {0x69, 0x0dcf8}}, 0x3, 0x0},
		{0x04,(0x04<<3)|0x0, {{0x6b, 0xdcf8}, {0x68, 0xdcf8}, {0x69, 0xdcf8}, {0x6a, 0x0dcf8}}, 0x4, 0x0},
		{0x04,(0x05<<3)|0x0, {{0x63, 0xdcd8}, {0x62, 0xdcf8}, {0x61, 0xdcf8}, {0x60, 0x0dcf8}}, 0x5, 0x0},
		{0x04,(0x06<<3)|0x0, {{0x62, 0xdcf8}, {0x61, 0xdcf8}, {0x60, 0xdcf8}, {0x63, 0x0dcd8}}, 0x6, 0x0},
		{0x04,(0x09<<3)|0x0, {{0x69, 0xdcf8}, {0x6a, 0xdcf8}, {0x6b, 0xdcf8}, {0x68, 0x0dcf8}}, 0x9, 0x0},
		{0x01,(0x00<<3)|0x0, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdcf8}, {0x63, 0x0dcd8}}, 0x0, 0x0}, // Ethernet 8168
		{0x02,(0x00<<3)|0x0, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdcf8}, {0x63, 0x0dcd8}}, 0x9, 0x0},
		{0x03,(0x00<<3)|0x0, {{0x61, 0xdcf8}, {0x62, 0xdcf8}, {0x63, 0xdcd8}, {0x60, 0x0dcf8}}, 0xa, 0x0},
	}
};

unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr);
}

