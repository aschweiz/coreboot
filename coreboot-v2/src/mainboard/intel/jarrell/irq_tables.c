/* PCI: Interrupt Routing Table found at 0x40114180 size = 320 */

#include <arch/pirq_routing.h>

const struct irq_routing_table intel_irq_routing_table = {
	0x52495024, /* u32 signature */
	0x0100,     /* u16 version   */
	320,        /* u16 Table size 32+(16*devices)  */
	0x00,       /* u8 Bus 0 */
	0xf8,       /* u8 Device 1, Function 0 */
	0x0000,     /* u16 reserve IRQ for PCI */
	0x8086,     /* u16 Vendor */
	0x24d0,     /* Device ID */
	0x00000000, /* u32 miniport_data */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, /* u8 rfu[11] */
	0x38,   /*  u8 checksum - mod 256 checksum must give zero */
	{  /* bus, devfn, {link, bitmap}, {link, bitmap}, {link, bitmap}, {link, bitmap}, slot, rfu  */
	    {0x00, 0x08, {{0x60, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x00,  0x00},
	    {0x00, 0xf8, {{0x62, 0xdc78}, {0x61, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x00,  0x00},
	    {0x00, 0xe8, {{0x60, 0xdcf8}, {0x63, 0xdcf8}, {0x62, 0xdc78}, {0x6b, 0xdcf8}}, 0x00,  0x00},
	    {0x02, 0x20, {{0x62, 0xdc78}, {0x63, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x00,  0x00},
	    {0x03, 0x28, {{0x62, 0xdc78}, {0x61, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x00,  0x00},
	    {0x04, 0x60, {{0x61, 0xdcf8}, {0x00, 0x0000}, {0x00, 0x0000}, {0x00, 0x0000}}, 0x00,  0x00},
	    {0x02, 0x08, {{0x60, 0xdcf8}, {0x63, 0xdcf8}, {0x62, 0xdc78}, {0x61, 0xdcf8}}, 0x04,  0x00},
	    {0x02, 0x10, {{0x63, 0xdcf8}, {0x62, 0xdc78}, {0x61, 0xdcf8}, {0x60, 0xdcf8}}, 0x05,  0x00},
	    {0x02, 0x18, {{0x62, 0xdc78}, {0x61, 0xdcf8}, {0x60, 0xdcf8}, {0x63, 0xdcf8}}, 0x06,  0x00},
	    {0x03, 0x08, {{0x60, 0xdcf8}, {0x63, 0xdcf8}, {0x61, 0xdcf8}, {0x60, 0xdcf8}}, 0x01,  0x00},
	    {0x03, 0x10, {{0x60, 0xdcf8}, {0x60, 0xdcf8}, {0x63, 0xdcf8}, {0x61, 0xdcf8}}, 0x02,  0x00},
	    {0x03, 0x18, {{0x60, 0xdcf8}, {0x63, 0xdcf8}, {0x62, 0xdc78}, {0x61, 0xdcf8}}, 0x03,  0x00},
	    {0x00, 0x10, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00},
	    {0x00, 0x18, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00},
	    {0x00, 0x20, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00},
	    {0x00, 0x28, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00},
	    {0x00, 0x30, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00},
	    {0x00, 0x38, {{0x60, 0xdcf8}, {0x61, 0xdcf8}, {0x62, 0xdc78}, {0x63, 0xdcf8}}, 0x00,  0x00}
	}
};

unsigned long write_pirq_routing_table(unsigned long addr)
{
	return copy_pirq_routing_table(addr);
}
