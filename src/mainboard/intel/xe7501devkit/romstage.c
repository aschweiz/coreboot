#include <stdint.h>
#include <device/pci_def.h>
#include <arch/io.h>
#include <device/pnp_def.h>
#include <arch/romcc_io.h>
#include <cpu/x86/lapic.h>
#include <arch/cpu.h>
#include <stdlib.h>
#include <pc80/mc146818rtc.h>
#include <console/console.h>
#include "lib/ramtest.c"
#include "southbridge/intel/i82801cx/i82801cx_early_smbus.c"
#include "northbridge/intel/e7501/raminit.h"
#include "cpu/x86/lapic/boot_cpu.c"
#include "northbridge/intel/e7501/debug.c"
#include "superio/smsc/lpc47b272/lpc47b272_early_serial.c"
#include "cpu/x86/mtrr/earlymtrr.c"
#include "cpu/x86/bist.h"

#define SUPERIO_PORT	0x2e
#define SERIAL_DEV		PNP_DEV(SUPERIO_PORT, LPC47B272_SP1)

static void hard_reset(void)
{
        outb(0x0e, 0x0cf9);
}

static inline int spd_read_byte(unsigned device, unsigned address)
{
	return smbus_read_byte(device, address);
}

#include "northbridge/intel/e7501/raminit.c"
#include "northbridge/intel/e7501/reset_test.c"
#include "lib/generic_sdram.c"

// This function MUST appear last (ROMCC limitation)
static void main(unsigned long bist)
{
	static const struct mem_controller memctrl[] = {
		{
			.d0 = PCI_DEV(0, 0, 0),
			.d0f1 = PCI_DEV(0, 0, 1),
			.channel0 = { (0xa<<3)|0, (0xa<<3)|1, (0xa<<3)|2, 0 },
			.channel1 = { (0xa<<3)|4, (0xa<<3)|5, (0xa<<3)|6, 0 },
		},
	};

	if (bist == 0)
	{
		// Skip this if there was a built in self test failure
		early_mtrr_init();
		enable_lapic();
	}

	// Get the serial port running and print a welcome banner

	lpc47b272_enable_serial(SERIAL_DEV, CONFIG_TTYS0_BASE);
	uart_init();
	console_init();

	// Halt if there was a built in self test failure
	report_bist_failure(bist);

	// print_pci_devices();

	// If this is a warm boot, some initialization can be skipped

	if (!bios_reset_detected())
	{
		enable_smbus();
		// dump_spd_registers(&memctrl[0]);
		// dump_smbus_registers();
		sdram_initialize(ARRAY_SIZE(memctrl), memctrl);
	}

	// NOTE: ROMCC dies with an internal compiler error
	//		 if the following line is removed.
	print_debug("SDRAM is up.\n");
}

