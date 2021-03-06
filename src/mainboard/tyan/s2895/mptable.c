#include <console/console.h>
#include <arch/smp/mpspec.h>
#include <device/pci.h>
#include <string.h>
#include <stdint.h>

#include <cpu/amd/amdk8_sysconf.h>

extern  unsigned char bus_isa;
extern  unsigned char bus_ck804_0; //1
extern  unsigned char bus_ck804_1; //2
extern  unsigned char bus_ck804_2; //3
extern  unsigned char bus_ck804_3; //4
extern  unsigned char bus_ck804_4; //5
extern  unsigned char bus_ck804_5; //6
extern  unsigned char bus_8131_0;  //7
extern  unsigned char bus_8131_1;  //8
extern  unsigned char bus_8131_2;  //9
extern  unsigned char bus_ck804b_0;//a
extern  unsigned char bus_ck804b_1;//b
extern  unsigned char bus_ck804b_2;//c
extern  unsigned char bus_ck804b_3;//d
extern  unsigned char bus_ck804b_4;//e
extern  unsigned char bus_ck804b_5;//f
extern  unsigned apicid_ck804;
extern  unsigned apicid_8131_1;
extern  unsigned apicid_8131_2;
extern  unsigned apicid_ck804b;

extern  unsigned sbdn3;
extern  unsigned sbdnb;



static void *smp_write_config_table(void *v)
{
	static const char sig[4] = "PCMP";
	static const char oem[8] = "COREBOOT";
	static const char productid[12] = "S2895       ";
	struct mp_config_table *mc;
	unsigned sbdn;

	unsigned char bus_num;
	int i;

	mc = (void *)(((char *)v) + SMP_FLOATING_TABLE_LEN);
	memset(mc, 0, sizeof(*mc));

	memcpy(mc->mpc_signature, sig, sizeof(sig));
	mc->mpc_length = sizeof(*mc); /* initially just the header */
	mc->mpc_spec = 0x04;
	mc->mpc_checksum = 0; /* not yet computed */
	memcpy(mc->mpc_oem, oem, sizeof(oem));
	memcpy(mc->mpc_productid, productid, sizeof(productid));
	mc->mpc_oemptr = 0;
	mc->mpc_oemsize = 0;
	mc->mpc_entry_count = 0; /* No entries yet... */
	mc->mpc_lapic = LAPIC_ADDR;
	mc->mpe_length = 0;
	mc->mpe_checksum = 0;
	mc->reserved = 0;

	smp_write_processors(mc);

	get_bus_conf();
	sbdn = sysconf.sbdn;

/*Bus:		Bus ID	Type*/
       /* define bus and isa numbers */
	for(bus_num = 0; bus_num < bus_isa; bus_num++) {
		smp_write_bus(mc, bus_num, "PCI   ");
	}
	smp_write_bus(mc, bus_isa, "ISA   ");

/*I/O APICs:	APIC ID	Version	State		Address*/
	{
		device_t dev;
		struct resource *res;
		uint32_t dword;

		dev = dev_find_slot(bus_ck804_0, PCI_DEVFN(sbdn+ 0x1,0));
		if (dev) {
			res = find_resource(dev, PCI_BASE_ADDRESS_1);
			if (res) {
				smp_write_ioapic(mc, apicid_ck804, 0x11, res->base);
			}

	/* Initialize interrupt mapping*/

			dword = 0x0120d218;
			pci_write_config32(dev, 0x7c, dword);

			dword = 0x12008a00;
			pci_write_config32(dev, 0x80, dword);

			dword = 0x00080d7d;
			pci_write_config32(dev, 0x84, dword);

		}

		dev = dev_find_slot(bus_8131_0, PCI_DEVFN(sbdn3,1));
		if (dev) {
			res = find_resource(dev, PCI_BASE_ADDRESS_0);
			if (res) {
				smp_write_ioapic(mc, apicid_8131_1, 0x11, res->base);
			}
		}
		dev = dev_find_slot(bus_8131_0, PCI_DEVFN(sbdn3+1,1));
		if (dev) {
			res = find_resource(dev, PCI_BASE_ADDRESS_0);
			if (res) {
				smp_write_ioapic(mc, apicid_8131_2, 0x11, res->base);
			}
		}

	    if(sysconf.pci1234[2] & 0xf) {
		dev = dev_find_slot(bus_ck804b_0, PCI_DEVFN(sbdnb + 0x1,0));
		if (dev) {
			res = find_resource(dev, PCI_BASE_ADDRESS_1);
			if (res) {
				smp_write_ioapic(mc, apicid_ck804b, 0x11, res->base);
			}

			dword = 0x0000d218; // Why does the factory BIOS have 0?
			pci_write_config32(dev, 0x7c, dword);

			dword = 0x00000000;
			pci_write_config32(dev, 0x80, dword);

			dword = 0x00000d00; // Same here.
			pci_write_config32(dev, 0x84, dword);

		}
	    }

	}

	mptable_add_isa_interrupts(mc, bus_isa, apicid_ck804, 1);

/*I/O Ints:	Type	Polarity    Trigger	Bus ID	 IRQ	APIC ID	PIN# */
// Onboard ck804 smbus
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn+1)<<2)|1, apicid_ck804, 0xa);
// 10

// Onboard ck804 USB 1.1
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn+2)<<2)|0, apicid_ck804, 0x15); // 21

// Onboard ck804 USB 2
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn+2)<<2)|1, apicid_ck804, 0x14); // 20

// Onboard ck804 Audio
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn+4)<<2)|0, apicid_ck804, 0x14); // 20

// Onboard ck804 SATA 0
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn +7)<<2)|0, apicid_ck804, 0x17); // 23

// Onboard ck804 SATA 1
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn +8)<<2)|0, apicid_ck804, 0x16); // 22

// Onboard ck804 NIC
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_0, ((sbdn +0x0a)<<2)|0, apicid_ck804, 0x15); // 21

//Slot PCIE x16
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_5, (0x00<<2)|i, apicid_ck804, 0x10 + (2+i+4-sbdn%4)%4);
	}

//Onboard Firewire
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_1, (0x05<<2)|0, apicid_ck804, 0x13); // 19

//Slot 2 PCI 32
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804_1, (0x04<<2)|i, apicid_ck804, 0x10 + (0+i)%4); //16
	}

	if(sysconf.pci1234[2] & 0xf) {
//Onboard ck804b NIC
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804b_0, ((sbdnb+0x0a)<<2)|0, apicid_ck804b, 0x15);//24+4+4+21=53

//Slot 3 PCIE x16
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_ck804b_5, (0x00<<2)|i, apicid_ck804b, 0x10 + (2+i+4-sbdnb%4)%4);
	}
	}

//Channel B of 8131

//Slot 4 PCI-X 100/66
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, (4<<2)|i, apicid_8131_2, (0+i)%4);
	}

//Slot 5 PCIX 100/66
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, (9<<2)|i, apicid_8131_2, (1+i)%4); // 29
	}

//OnBoard LSI SCSI
	for(i=0;i<2;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, (6<<2)|i, apicid_8131_2, (2+i)%4); //30
	}

//Channel A of 8131

//Slot 6 PCIX 133/100/66
	for(i=0;i<4;i++) {
		smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_1, (4<<2)|i, apicid_8131_1, (0+i)%4); //24
	}

/*Local Ints:	Type	Polarity    Trigger	Bus ID	 IRQ	APIC ID	PIN#*/
	smp_write_intsrc(mc, mp_ExtINT, MP_IRQ_TRIGGER_EDGE|MP_IRQ_POLARITY_HIGH, bus_isa, 0x0, MP_APIC_ALL, 0x0);
	smp_write_intsrc(mc, mp_NMI, MP_IRQ_TRIGGER_EDGE|MP_IRQ_POLARITY_HIGH, bus_isa, 0x0, MP_APIC_ALL, 0x1);
	/* There is no extension information... */

	/* Compute the checksums */
	mc->mpe_checksum = smp_compute_checksum(smp_next_mpc_entry(mc), mc->mpe_length);
	mc->mpc_checksum = smp_compute_checksum(mc, mc->mpc_length);
	printk(BIOS_DEBUG, "Wrote the mp table end at: %p - %p\n",
		mc, smp_next_mpe_entry(mc));
	return smp_next_mpe_entry(mc);
}

unsigned long write_smp_table(unsigned long addr)
{
	void *v;
	v = smp_write_floating_table(addr);
	return (unsigned long)smp_write_config_table(v);
}
