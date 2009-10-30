#include "amd8111_smbus.h"

#define SMBUS_IO_BASE 0x0f00

static void enable_smbus(void)
{
	device_t dev;
	uint8_t enable;

	dev = pci_locate_device(PCI_ID(0x1022, 0x746b), 0);
	if (dev == PCI_DEV_INVALID) {
		die("SMBUS controller not found\r\n");
	}

	pci_write_config32(dev, 0x58, SMBUS_IO_BASE | 1);
	enable = pci_read_config8(dev, 0x41);
	pci_write_config8(dev, 0x41, enable | (1 << 7));

	/* check that we can see the smbus controller I/O. */
	if (inw(SMBUS_IO_BASE)==0xFF){
		die("SMBUS controller I/O not found\n");
	}

	/* clear any lingering errors, so the transaction will run */
	outw(inw(SMBUS_IO_BASE + SMBGSTATUS), SMBUS_IO_BASE + SMBGSTATUS);
	print_spew("SMBus controller enabled\r\n");
}

static int smbus_recv_byte(unsigned device)
{
	return do_smbus_recv_byte(SMBUS_IO_BASE, device);
}

static int smbus_send_byte(unsigned device, unsigned char val)
{
	return do_smbus_send_byte(SMBUS_IO_BASE, device, val);
}

static int smbus_read_byte(unsigned device, unsigned address)
{
	return do_smbus_read_byte(SMBUS_IO_BASE, device, address);
}

static int smbus_write_byte(unsigned device, unsigned address, unsigned char val)
{
	return do_smbus_write_byte(SMBUS_IO_BASE, device, address, val);
}