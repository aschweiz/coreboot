/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2009 One Laptop per Child, Association, Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <spd.h>
#include <sdram_mode.h>
#include <delay.h>
#include <arch/pci_rawops.h>

#define DEBUG_RAM_SETUP 1


#ifdef DEBUG_RAM_SETUP
#define PRINT_DEBUG_MEM(x)		print_debug(x)
#define PRINT_DEBUG_MEM_HEX8(x)		print_debug_hex8(x)
#define PRINT_DEBUG_MEM_HEX16(x)	print_debug_hex16(x)
#define PRINT_DEBUG_MEM_HEX32(x)	print_debug_hex32(x)
#define DUMPNORTH()			dump_pci_device(PCI_DEV(0, 0, 0))
#else
#define PRINT_DEBUG_MEM(x)
#define PRINT_DEBUG_MEM_HEX8(x)
#define PRINT_DEBUG_MEM_HEX16(x)
#define PRINT_DEBUG_MEM_HEX32(x)
#define DUMPNORTH()
#endif
#include "northbridge/via/vx800/ddr2init/translator_ddr2_init.c"
#include "northbridge/via/vx800/ddr2init/dram_init.h"
#include "northbridge/via/vx800/vx800_early_smbus.c"
#include "northbridge/via/vx800/vx800_early_serial.c"
#include "northbridge/via/vx800/ddr2init/dram_util.h"
#include "northbridge/via/vx800/ddr2init/dram_util.c"
#include "northbridge/via/vx800/ddr2init/vx800/detection.c"
#include "northbridge/via/vx800/ddr2init/vx800/freq_setting.c"
#include "northbridge/via/vx800/ddr2init/vx800/timing_setting.c"
#include "northbridge/via/vx800/ddr2init/vx800/drdy_bl.c"
#include "northbridge/via/vx800/ddr2init/vx800/driving_setting.c"
#include "northbridge/via/vx800/ddr2init/vx800/clk_ctrl.c"
#include "northbridge/via/vx800/ddr2init/vx800/dev_init.c"
#include "northbridge/via/vx800/ddr2init/vx800/rank_map.c"
#include "northbridge/via/vx800/ddr2init/vx800/dqs_search.c"
#include "northbridge/via/vx800/ddr2init/vx800/final_setting.c"
#include "northbridge/via/vx800/ddr2init/vx800/uma_ram_setting.c"
#include "northbridge/via/vx800/ddr2init/dram_init.c"
/*
 * Support one dimm with up to 2 ranks
 */

static void ddr2_ram_setup()
{
	u8 Data;
	CB_STATUS Status;
	PRINT_DEBUG_MEM("In ddr2_ram_setup\r");

	Status = DDR2_DRAM_INIT();
	if (CB_SUCCESS != Status) {
		PRINT_DEBUG_MEM("Dram init error. Status = %x\r");
	}

}