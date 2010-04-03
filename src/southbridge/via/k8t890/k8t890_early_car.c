/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007 Rudolf Marek <r.marek@assembler.cz>
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

/*
 * Seems the link and width of HT link needs to be setup too, you need to
 * generate PCI reset or LDTSTOP to apply.
 */

#include <stdlib.h>
//include "k8t890.h"
#warning hack the right header here

/* The 256 bytes of NVRAM for S3 storage, 256B aligned */
#define K8T890_NVRAM_IO_BASE	0xf00
#define K8T890_MULTIPLE_FN_EN	0x4f
/* we provide S3 NVRAM to system */
#define S3_NVRAM_EARLY	1


/* AMD K8 LDT0, LDT1, LDT2 Link Control Registers */
static ldtreg[3] = {0x86, 0xa6, 0xc6};

/* This functions sets KT890 link frequency and width to same values as
 * it has been setup on K8 side, by AMD NB init.
 */ 

u8 k8t890_early_setup_ht(void)
{
	u8 awidth, afreq, cldtfreq, reg;
	u8 cldtwidth_in, cldtwidth_out, vldtwidth_in, vldtwidth_out, ldtnr, width;
	u16 vldtcaps;

	/* hack, enable NVRAM in chipset */
	pci_write_config8(PCI_DEV(0, 0x0, 0), K8T890_MULTIPLE_FN_EN, 0x01);

	/*
	 * NVRAM I/O base at K8T890_NVRAM_IO_BASE
	 */

	pci_write_config8(PCI_DEV(0, 0x0, 2), 0xa2, (K8T890_NVRAM_IO_BASE >> 8));
	reg = pci_read_config8(PCI_DEV(0, 0x0, 2), 0xa1);
	reg |= 0x1;
	pci_write_config8(PCI_DEV(0, 0x0, 2), 0xa1, reg);

	/* check if connected non coherent, initcomplete (find the SB on K8 side) */
	ldtnr = 0;
	if (0x7 == pci_read_config8(PCI_DEV(0, 0x18, 0), 0x98)) {
		ldtnr = 0;
	} else if (0x7 == pci_read_config8(PCI_DEV(0, 0x18, 0), 0xb8)) {
		ldtnr = 1;
	} else if (0x7 == pci_read_config8(PCI_DEV(0, 0x18, 0), 0xd8)) {
		ldtnr = 2;
	}

	print_debug("K8T890 found at LDT ");
	print_debug_hex8(ldtnr);

	/* get the maximum widths for both sides */
	cldtwidth_in = pci_read_config8(PCI_DEV(0, 0x18, 0), ldtreg[ldtnr]) & 0x7;
	cldtwidth_out = (pci_read_config8(PCI_DEV(0, 0x18, 0), ldtreg[ldtnr]) >> 4) & 0x7;
	vldtwidth_in = pci_read_config8(PCI_DEV(0, 0x0, 0), 0x66) & 0x7;
	vldtwidth_out = (pci_read_config8(PCI_DEV(0, 0x0, 0), 0x66) >> 4) & 0x7;

	width = MIN(MIN(MIN(cldtwidth_out, cldtwidth_in), vldtwidth_out), vldtwidth_in);
	print_debug(" Agreed on width: ");
	print_debug_hex8(width);

	awidth = pci_read_config8(PCI_DEV(0, 0x0, 0), 0x67);

	/* Update the desired HT LNK to match AMD NB max from VIA NB is 0x1 */
	width = (width == 0x01) ? 0x11 : 0x00;

	pci_write_config8(PCI_DEV(0, 0x0, 0), 0x67, width);

	/* Get programmed HT freq at base 0x89 */
	cldtfreq = pci_read_config8(PCI_DEV(0, 0x18, 0), ldtreg[ldtnr] + 3) & 0xf;
	print_debug(" CPU programmed to HT freq: ");
	print_debug_hex8(cldtfreq);

	print_debug(" VIA HT caps: ");
	vldtcaps = pci_read_config16(PCI_DEV(0, 0, 0), 0x6e);
	print_debug_hex16(vldtcaps);

	if (!(vldtcaps & (1 << cldtfreq ))) {
		die("Chipset does not support desired HT frequency\n");
	}

	afreq = pci_read_config8(PCI_DEV(0, 0x0, 0), 0x6d);
	pci_write_config8(PCI_DEV(0, 0x0, 0), 0x6d, cldtfreq);
	print_debug("\n");

	/* no reset needed */
	if ((width == awidth) && (afreq == cldtfreq)) {
		return 0;
	}

	return 1;
}

int s3_save_nvram_early(u32 dword, int size, int  nvram_pos) {
	
	printk(BIOS_DEBUG, "Writing %x of size %d to nvram pos: %d\n", dword, size, nvram_pos);
	switch (size) {
	case 1:
		outb((dword & 0xff), K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=1;
		break;
	case 2:
		outw((dword & 0xffff), K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=2;
		break;
	default:
		outl(dword, K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=4;
		break;
	}
	return nvram_pos;
}

int s3_load_nvram_early(int size, u32 *old_dword, int nvram_pos) {
	switch (size) {
	case 1:
		*old_dword &= ~0xff;
		*old_dword |= inb(K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=1;
		break;
	case 2:
		*old_dword &= ~0xffff;
		*old_dword |= inw(K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=2;
		break;
	default:
		*old_dword = inl(K8T890_NVRAM_IO_BASE+nvram_pos);
		nvram_pos +=4;
		break;
	}
	printk(BIOS_DEBUG, "Loading %x of size %d to nvram pos:%d\n", * old_dword, size, nvram_pos-size);
	return nvram_pos;
}
