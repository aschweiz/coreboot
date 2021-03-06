#include <console/console.h>
#include <arch/smp/mpspec.h>
#include <arch/ioapic.h>
#include <device/pci.h>
#include <string.h>
#include <stdint.h>

#include <cpu/amd/amdk8_sysconf.h>

extern  unsigned char bus_isa;
extern  unsigned char bus_8131_0;
extern  unsigned char bus_8131_1;
extern  unsigned char bus_8131_2;
extern  unsigned char bus_8111_0;
extern  unsigned char bus_8111_1;
extern  unsigned apicid_8111;
extern  unsigned apicid_8131_1;
extern  unsigned apicid_8131_2;

extern  unsigned sbdn3;



static void *smp_write_config_table(void *v)
{
        static const char sig[4] = "PCMP";
        static const char oem[8] = "COREBOOT";
        static const char productid[12] = "DL145G1     ";
        struct mp_config_table *mc;

        unsigned char bus_num;

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


/*Bus:          Bus ID  Type*/
        /* define bus and isa numbers */
        for(bus_num = 0; bus_num < bus_isa; bus_num++) {
                smp_write_bus(mc, bus_num, "PCI   ");
        }
        smp_write_bus(mc, bus_isa, "ISA   ");


/*I/O APICs:	APIC ID	Version	State		Address*/
	smp_write_ioapic(mc, apicid_8111, 0x20, IO_APIC_ADDR);
        {
                device_t dev;
                struct resource *res;
                dev = dev_find_slot(bus_8131_0, PCI_DEVFN(sbdn3,1));
                if (dev) {
                        res = find_resource(dev, PCI_BASE_ADDRESS_0);
                        if (res) {
                                smp_write_ioapic(mc, apicid_8131_1, 0x20, res->base);
                        }
                }
                dev = dev_find_slot(bus_8131_0, PCI_DEVFN(sbdn3+1,1));
                if (dev) {
                        res = find_resource(dev, PCI_BASE_ADDRESS_0);
                        if (res) {
                                smp_write_ioapic(mc, apicid_8131_2, 0x20, res->base);
                        }
                }

	}

	mptable_add_isa_interrupts(mc, bus_isa, apicid_8111, 0);

	//
	// The commented-out lines are auto-detected on my servers.
	//
/*I/O Ints:	Type	Polarity    Trigger	Bus ID	 IRQ	APIC ID	PIN# */
	// Integrated SMBus 2.0
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_0, ( 0x4 <<2)|3, apicid_8111  , 0x15);
	// Integrated AMD AC97 Audio
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_0, ( 0x4 <<2)|1, apicid_8111  , 0x11);
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_0, ( 0x4 <<2)|2, apicid_8111  , 0x12);
	// Integrated AMD USB
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_1, ( 0x4 <<2)|0, apicid_8111  , 0x10);
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_1, ( 0x0 <<2)|3, apicid_8111  , 0x13);
	// On board ATI Rage XL
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8111_1, ( 0x5 <<2)|0, apicid_8111  , 0x14);
	// On board Broadcom nics
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, ( 0x3 <<2)|0, apicid_8131_2, 0x03);
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, ( 0x3 <<2)|1, apicid_8131_2, 0x00);
	// On board LSI SCSI
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_2, ( 0x2 <<2)|0, apicid_8131_2, 0x02);

	// PCIX-133 Slot
	smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_1, ( 0x1 <<2)|0, apicid_8131_1, 0x01);
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_1, ( 0x1 <<2)|1, apicid_8131_1, 0x02);
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_1, ( 0x1 <<2)|2, apicid_8131_1, 0x03);
	//smp_write_intsrc(mc, mp_INT, MP_IRQ_TRIGGER_LEVEL|MP_IRQ_POLARITY_LOW, bus_8131_1, ( 0x1 <<2)|3, apicid_8131_1, 0x04);

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
