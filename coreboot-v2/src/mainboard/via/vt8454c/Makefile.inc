##
## This file is part of the coreboot project.
## 
## Copyright (C) 2007-2009 coresystems GmbH
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; version 2 of
## the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
## MA 02110-1301 USA
##

##
## This mainboard requires DCACHE_AS_RAM enabled. It won't work without.
## 

driver-y +=  mainboard.o

obj-$(CONFIG_HAVE_MP_TABLE) += mptable.o
obj-$(CONFIG_HAVE_PIRQ_TABLE) += irq_tables.o
obj-$(CONFIG_HAVE_ACPI_TABLES) +=  dsdt.o
obj-$(CONFIG_HAVE_ACPI_TABLES) +=  acpi_tables.o
obj-$(CONFIG_HAVE_ACPI_TABLES) +=  fadt.o

initobj-y += crt0.o
# FIXME in $(top)/Makefile
crt0-y += ../../../../src/cpu/x86/16bit/entry16.inc
crt0-y += ../../../../src/cpu/x86/32bit/entry32.inc
crt0-y += ../../../../src/cpu/x86/16bit/reset16.inc
crt0-y += ../../../../src/arch/i386/lib/id.inc
crt0-y += ../../../../src/cpu/via/car/cache_as_ram.inc
crt0-y += auto.inc

ldscript-y += ../../../../src/arch/i386/init/ldscript_fallback_cbfs.lb
ldscript-y += ../../../../src/cpu/x86/16bit/entry16.lds
ldscript-y += ../../../../src/cpu/x86/16bit/reset16.lds
ldscript-y += ../../../../src/arch/i386/lib/id.lds
ldscript-y += ../../../../src/arch/i386/lib/failover.lds

ifdef POST_EVALUATION

$(obj)/dsdt.c: $(src)/mainboard/$(MAINBOARDDIR)/dsdt.asl
	iasl -p dsdt -tc $(src)/mainboard/$(MAINBOARDDIR)/dsdt.asl
	mv dsdt.hex $@

$(obj)/mainboard/$(MAINBOARDDIR)/dsdt.o: $(obj)/dsdt.c
	$(CC) $(DISTRO_CFLAGS) $(CFLAGS) $(CPPFLAGS) $(DEBUG_CFLAGS) -I$(src) -I. -c $< -o $@

$(obj)/mainboard/$(MAINBOARDDIR)/auto.inc: $(src)/mainboard/$(MAINBOARDDIR)/auto.c $(obj)/option_table.h
	$(CC) $(DISTRO_CFLAGS) $(CFLAGS) $(CPPFLAGS) $(DEBUG_CFLAGS) -I$(src) -I. -c -S  $(src)/mainboard/$(MAINBOARDDIR)/auto.c -o $@
	perl -e 's/\.rodata/.rom.data/g' -pi $@
	perl -e 's/\.text/.section .rom.text/g' -pi $@

endif
