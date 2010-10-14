/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2010 coresystems GmbH
 * Copyright (C) 2010 Warren Turkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

// __PRE_RAM__ means: use "unsigned" for device, not a struct.

/* Configuration of the i945 driver */
#define CHIPSET_I945GM 1
/* Usually system firmware turns off system memory clock signals to
 * unused SO-DIMM slots to reduce EMI and power consumption.
 * However, the Kontron 986LCD-M does not like unused clock signals to
 * be disabled. If other similar mainboard occur, it would make sense
 * to make this an entry in the sysinfo structure, and pre-initialize that
 * structure in the mainboard's romstage.c main() function. For now a
 * #define will do.
 */
#define OVERRIDE_CLOCK_DISABLE 1
#define CHANNEL_XOR_RANDOMIZATION 1

#include <stdint.h>
#include <string.h>
#include <arch/io.h>
#include <arch/romcc_io.h>
#include <device/pci_def.h>
#include <device/pnp_def.h>
#include <cpu/x86/lapic.h>

#include "superio/winbond/w83627thg/w83627thg.h"

#include <pc80/mc146818rtc.h>

#include <console/console.h>
#include <cpu/x86/bist.h>

#if CONFIG_USBDEBUG
#define DBGP_DEFAULT 1
#include <usbdebug.h>
#include "southbridge/intel/i82801jir/i82801jir_usb_debug.c"
#include "pc80/usbdebug_serial.c"
#endif

//#include "lib/ramtest.c"
#include "lib.h"
#include "southbridge/intel/i82801jir/i82801jir_early_smbus.c"
//#include "superio/winbond/w83627thg/w83627thg_early_serial.c"

#include "northbridge/intel/core_i7/udelay.c"

//#define SERIAL_DEV PNP_DEV(0x2e, W83627THG_SP1)

#include "southbridge/intel/i82801jir/i82801jir.h"
static void setup_ich10_gpios(void)
{
	printk(BIOS_DEBUG, " GPIOS...");
#if 0
	/* General Registers */
	outl(0x1f1ff7c0, DEFAULT_GPIOBASE + 0x00);	/* GPIO_USE_SEL */
	outl(0xe0e8efc3, DEFAULT_GPIOBASE + 0x04);	/* GP_IO_SEL */
	outl(0xebffeeff, DEFAULT_GPIOBASE + 0x0c);	/* GP_LVL */
	/* Output Control Registers */
	outl(0x00000000, DEFAULT_GPIOBASE + 0x18);	/* GPO_BLINK */
	/* Input Control Registers */
	outl(0x00002180, DEFAULT_GPIOBASE + 0x2c);	/* GPI_INV */
	outl(0x000100ff, DEFAULT_GPIOBASE + 0x30);	/* GPIO_USE_SEL2 */
	outl(0x00000030, DEFAULT_GPIOBASE + 0x34);	/* GP_IO_SEL2 */
	outl(0x00010035, DEFAULT_GPIOBASE + 0x38);	/* GP_LVL */
#endif
}

#include "northbridge/intel/core_i7/early_init.c"

static inline int spd_read_byte(unsigned device, unsigned address)
{
	return smbus_read_byte(device, address);
}

#include "northbridge/intel/core_i7/raminit.h"
#include "northbridge/intel/core_i7/raminit.c"
#include "northbridge/intel/core_i7/errata.c"
#include "northbridge/intel/core_i7/debug.c"

static void ich10_enable_lpc(void)
{
#if 0
	// Enable Serial IRQ
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0x64, 0xd0);
	// Set COM1/COM2 decode range
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x80, 0x0010);
	// Enable COM1/COM2/KBD/SuperIO1+2
	pci_write_config16(PCI_DEV(0, 0x1f, 0), 0x82, 0x340b);
	// Enable HWM at 0xa00
	pci_write_config32(PCI_DEV(0, 0x1f, 0), 0x84, 0x00fc0a01);
	// COM3 decode
	pci_write_config32(PCI_DEV(0, 0x1f, 0), 0x88, 0x000403e9);
	// COM4 decode
	pci_write_config32(PCI_DEV(0, 0x1f, 0), 0x8c, 0x000402e9);
#endif

	// io 0x300 decode
	pci_write_config32(PCI_DEV(0, 0x1f, 0), 0x90, 0x00000301);
}

/* This box has two superios, so enabling serial becomes slightly excessive.
 * We disable a lot of stuff to make sure that there are no conflicts between
 * the two. Also set up the GPIOs from the beginning. This is the "no schematic
 * but safe anyways" method.
 */
static void early_superio_config_w83627thg(void)
{
#if 0
	device_t dev;

	dev=PNP_DEV(0x2e, W83627THG_SP1);
	pnp_enter_ext_func_mode(dev);

	pnp_write_config(dev, 0x24, 0xc6); // PNPCSV

	pnp_write_config(dev, 0x29, 0x43); // GPIO settings
	pnp_write_config(dev, 0x2a, 0x40); // GPIO settings

	dev=PNP_DEV(0x2e, W83627THG_SP1);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x3f8);
	pnp_set_irq(dev, PNP_IDX_IRQ0, 4);
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x2e, W83627THG_SP2);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x2f8);
	pnp_set_irq(dev, PNP_IDX_IRQ0, 3);
	// pnp_write_config(dev, 0xf1, 4); // IRMODE0
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x2e, W83627THG_KBC);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x60);
	pnp_set_iobase(dev, PNP_IDX_IO1, 0x64);
	// pnp_write_config(dev, 0xf0, 0x82);
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x2e, W83627THG_GAME_MIDI_GPIO1);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_write_config(dev, 0xf5, 0xff); // invert all GPIOs
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x2e, W83627THG_GPIO2);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 1); // Just enable it

	dev=PNP_DEV(0x2e, W83627THG_GPIO3);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_write_config(dev, 0xf0, 0xfb); // GPIO bit 2 is output
	pnp_write_config(dev, 0xf1, 0x00); // GPIO bit 2 is 0
	pnp_write_config(dev, 0x30, 0x03); // Enable GPIO3+4. pnp_set_enable is not sufficient

	dev=PNP_DEV(0x2e, W83627THG_FDC);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);

	dev=PNP_DEV(0x2e, W83627THG_PP);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);

	/* Enable HWM */
	dev=PNP_DEV(0x2e, W83627THG_HWM);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0xa00);
	pnp_set_enable(dev, 1);

	pnp_exit_ext_func_mode(dev);

	dev=PNP_DEV(0x4e, W83627THG_SP1);
	pnp_enter_ext_func_mode(dev);

	pnp_set_logical_device(dev); // Set COM3 to sane non-conflicting values
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x3e8);
	pnp_set_irq(dev, PNP_IDX_IRQ0, 11);
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x4e, W83627THG_SP2);
	pnp_set_logical_device(dev); // Set COM4 to sane non-conflicting values
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x2e8);
	pnp_set_irq(dev, PNP_IDX_IRQ0, 10);
	pnp_set_enable(dev, 1);

	dev=PNP_DEV(0x4e, W83627THG_FDC);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);

	dev=PNP_DEV(0x4e, W83627THG_PP);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);

	dev=PNP_DEV(0x4e, W83627THG_KBC);
	pnp_set_logical_device(dev);
	pnp_set_enable(dev, 0);
	pnp_set_iobase(dev, PNP_IDX_IO0, 0x00);
	pnp_set_iobase(dev, PNP_IDX_IO1, 0x00);

	pnp_exit_ext_func_mode(dev);
#endif
}

static void rcba_config(void)
{
	//u32 reg32;

	/* Set up virtual channel 0 */
	RCBA32(0x0014) = 0x80000001;
	RCBA32(0x001c) = 0x03128010;

	/* Device 1f interrupt pin register */
	RCBA32(0x3100) = 0x03243200;
	/* Device 1e interrupt pin register */
	RCBA32(0x3108) = 0x10004321;
	/* Device 1d interrupt pin register */
	RCBA32(0x310c) = 0x00214321;

	/* dev irq route register */
//	RCBA16(0x3140) = 0x0132;
//	RCBA16(0x3142) = 0x3241;
//	RCBA16(0x3144) = 0x0237;
//	RCBA16(0x3146) = 0x3210;
//	RCBA16(0x3148) = 0x3210;
	RCBA16(0x3140) = 0x3210;
//	RCBA16(0x3142) = 0x0000;
	RCBA16(0x3144) = 0x3210;
	RCBA16(0x3146) = 0x3210;
	RCBA16(0x3148) = 0x3210;
	RCBA16(0x314c) = 0x3210;
	RCBA16(0x3150) = 0x3210;

	/* Enable IOAPIC */
	//RCBA8(0x31ff) = 0x03;
	RCBA8(0x31ff) = (1 << 0);

	/* Enable upper 128bytes of CMOS */
	RCBA32(0x3400) = (1 << 2);
}

static void early_ich10_init(void)
{
#if 0
	uint8_t reg8;
	uint32_t reg32;

	// program secondary mlt XXX byte?
	pci_write_config8(PCI_DEV(0, 0x1e, 0), 0x1b, 0x20);

	// reset rtc power status
	reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa4);
	reg8 &= ~(1 << 2);
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa4, reg8);

	// usb transient disconnect
	reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xad);
	reg8 |= (3 << 0);
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xad, reg8);

	reg32 = pci_read_config32(PCI_DEV(0, 0x1d, 7), 0xfc);
	reg32 |= (1 << 29) | (1 << 17);
	pci_write_config32(PCI_DEV(0, 0x1d, 7), 0xfc, reg32);

	reg32 = pci_read_config32(PCI_DEV(0, 0x1d, 7), 0xdc);
	reg32 |= (1 << 31) | (1 << 27);
	pci_write_config32(PCI_DEV(0, 0x1d, 7), 0xdc, reg32);

	RCBA32(0x0088) = 0x0011d000;
	RCBA16(0x01fc) = 0x060f;
	RCBA32(0x01f4) = 0x86000040;
	RCBA32(0x0214) = 0x10030549;
	RCBA32(0x0218) = 0x00020504;
	RCBA8(0x0220) = 0xc5;
	reg32 = RCBA32(0x3410);
	reg32 |= (1 << 6);
	RCBA32(0x3410) = reg32;
	reg32 = RCBA32(0x3430);
	reg32 &= ~(3 << 0);
	reg32 |= (1 << 0);
	RCBA32(0x3430) = reg32;
	RCBA32(0x3418) |= (1 << 0);
	RCBA16(0x0200) = 0x2008;
	RCBA8(0x2027) = 0x0d;
	RCBA16(0x3e08) |= (1 << 7);
	RCBA16(0x3e48) |= (1 << 7);
	RCBA32(0x3e0e) |= (1 << 7);
	RCBA32(0x3e4e) |= (1 << 7);

	// next step only on ich7m b0 and later:
	reg32 = RCBA32(0x2034);
	reg32 &= ~(0x0f << 16);
	reg32 |= (5 << 16);
	RCBA32(0x2034) = reg32;
#endif
}

#include <cbmem.h>

// Now, this needs to be included because it relies on the symbol
// __PRE_RAM__ being set during CAR stage (in order to compile the
// BSS free versions of the functions). Either rewrite the code
// to be always BSS free, or invent a flag that's better suited than
// __PRE_RAM__ to determine whether we're in ram init stage (stage 1)
//
#include "lib/cbmem.c"

void main(unsigned long bist)
{
//	u32 reg32;
//	int boot_mode = 0;

	if (bist == 0) {
		enable_lapic();
	}

	/* Force PCIRST# */
	pci_write_config16(PCI_DEV(0, 0x1e, 0), BCTRL, SBR);
	udelay(200 * 1000);
	pci_write_config16(PCI_DEV(0, 0x1e, 0), BCTRL, 0);

	ich10_enable_lpc();

	early_superio_config_w83627thg();

#if 0
	/* Set up the console */
	uart_init();

#if CONFIG_USBDEBUG
	i82801jir_enable_usbdebug(DBGP_DEFAULT);
	early_usbdebug_init();
#endif

	console_init();
#endif

	/* Halt if there was a built in self test failure */
	report_bist_failure(bist);

#if 0
	if (MCHBAR16(SSKPD) == 0xCAFE) {
		printk(BIOS_DEBUG, "soft reset detected, rebooting properly\n");
		outb(0x6, 0xcf9);
		while (1) asm("hlt");
	}
#endif

	/* Perform some early chipset initialization required
	 * before RAM initialization can work
	 */
	core_i7_early_initialization();

	do {} while(1);

#if 0
	/* Read PM1_CNT */
	reg32 = inl(DEFAULT_PMBASE + 0x04);
	printk(BIOS_DEBUG, "PM1_CNT: %08x\n", reg32);
	if (((reg32 >> 10) & 7) == 5) {
#if CONFIG_HAVE_ACPI_RESUME
		printk(BIOS_DEBUG, "Resume from S3 detected.\n");
		boot_mode = 2;
		/* Clear SLP_TYPE. This will break stage2 but
		 * we care for that when we get there.
		 */
		outl(reg32 & ~(7 << 10), DEFAULT_PMBASE + 0x04);

#else
		printk(BIOS_DEBUG, "Resume from S3 detected, but disabled.\n");
#endif
	}
#endif

	/* Enable SPD ROMs and DDR-II DRAM */
	enable_smbus();

#if 0
#if CONFIG_DEFAULT_CONSOLE_LOGLEVEL > 8
	dump_spd_registers();
#endif

	sdram_initialize(boot_mode);
#endif

	/* Perform some initialization that must run before stage2 */
	early_ich10_init();

	/* This should probably go away. Until now it is required
	 * and mainboard specific
	 */
	rcba_config();

#if 0
	/* Chipset Errata! */
	fixup_i945_errata();
#endif

	/* Initialize the internal PCIe links before we go into stage2 */
	core_i7_late_initialization();

#if 0
#if !CONFIG_HAVE_ACPI_RESUME
#if CONFIG_DEFAULT_CONSOLE_LOGLEVEL > 8
#if CONFIG_DEBUG_RAM_SETUP
	sdram_dump_mchbar_registers();
#endif

	{
		/* This will not work if TSEG is in place! */
		u32 tom = pci_read_config32(PCI_DEV(0,2,0), 0x5c);

		printk(BIOS_DEBUG, "TOM: 0x%08x\n", tom);
		ram_check(0x00000000, 0x000a0000);
		//ram_check(0x00100000, tom);
	}
#endif
#endif

	quick_ram_check();

	MCHBAR16(SSKPD) = 0xCAFE;

#if CONFIG_HAVE_ACPI_RESUME
	/* Start address of high memory tables */
	unsigned long high_ram_base = get_top_of_ram() - HIGH_MEMORY_SIZE;

	/* If there is no high memory area, we didn't boot before, so
	 * this is not a resume. In that case we just create the cbmem toc.
	 */
	if ((boot_mode == 2) && cbmem_reinit((u64)high_ram_base)) {
		void *resume_backup_memory = cbmem_find(CBMEM_ID_RESUME);

		/* copy 1MB - 64K to high tables ram_base to prevent memory corruption
		 * through stage 2. We could keep stuff like stack and heap in high tables
		 * memory completely, but that's a wonderful clean up task for another
		 * day.
		 */
		if (resume_backup_memory)
			memcpy(resume_backup_memory, (void *)CONFIG_RAMBASE, HIGH_MEMORY_SAVE);

		/* Magic for S3 resume */
		pci_write_config32(PCI_DEV(0, 0x00, 0), SKPAD, 0xcafed00d);
	}
#endif
#endif

}
