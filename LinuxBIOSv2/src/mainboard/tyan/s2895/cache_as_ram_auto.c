#define ASSEMBLY 1
#define __ROMCC__


#define K8_ALLOCATE_IO_RANGE 1
//#define K8_SCAN_PCI_BUS 1


#define K8_4RANK_DIMM_SUPPORT 1

#if CONFIG_LOGICAL_CPUS==1
#define SET_NB_CFG_54 1
#endif

 
#include <stdint.h>
#include <device/pci_def.h>
#include <arch/io.h>
#include <device/pnp_def.h>
#include <arch/romcc_io.h>
#include <cpu/x86/lapic.h>
#include "option_table.h"
#include "pc80/mc146818rtc_early.c"
#include "pc80/serial.c"
#include "arch/i386/lib/console.c"
#include "ram/ramtest.c"

#include "northbridge/amd/amdk8/cpu_rev.c"
#include "northbridge/amd/amdk8/incoherent_ht.c"
#include "southbridge/nvidia/ck804/ck804_early_smbus.c"
#include "northbridge/amd/amdk8/raminit.h"
#include "cpu/amd/model_fxx/apic_timer.c"
#include "lib/delay.c"

#if CONFIG_USE_INIT == 0
#include "lib/memcpy.c"
#endif

#include "cpu/x86/lapic/boot_cpu.c"
#include "northbridge/amd/amdk8/reset_test.c"
#include "northbridge/amd/amdk8/debug.c"
#include "superio/smsc/lpc47b397/lpc47b397_early_serial.c"

#include "cpu/amd/mtrr/amd_earlymtrr.c"
#include "cpu/x86/bist.h"

#include "superio/smsc/lpc47b397/lpc47b397_early_gpio.c"

#include "northbridge/amd/amdk8/setup_resource_map.c"

#define SERIAL_DEV PNP_DEV(0x2e, LPC47B397_SP1)

static void hard_reset(void)
{
        set_bios_reset();

        /* full reset */
	outb(0x0a, 0x0cf9);
        outb(0x0e, 0x0cf9);
}

static void soft_reset(void)
{
        set_bios_reset();
#if 1
        /* link reset */
	outb(0x02, 0x0cf9);
        outb(0x06, 0x0cf9);
#endif
}

static void memreset_setup(void)
{
}

static void memreset(int controllers, const struct mem_controller *ctrl)
{
}

#define SUPERIO_GPIO_DEV PNP_DEV(0x2e, LPC47B397_RT)

#define SUPERIO_GPIO_IO_BASE 0x400

static void sio_gpio_setup(void){

        unsigned value;

        /*Enable onboard scsi*/
        lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x2c, (1<<7)|(0<<2)|(0<<1)|(0<<0)); // GP21, offset 0x2c, DISABLE_SCSI_L 
        value = lpc47b397_gpio_offset_in(SUPERIO_GPIO_IO_BASE, 0x4c);
        lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x4c, (value|(1<<1)));

}

static inline void activate_spd_rom(const struct mem_controller *ctrl)
{
	/* nothing to do */
}

static inline int spd_read_byte(unsigned device, unsigned address)
{
	return smbus_read_byte(device, address);
}


#include "northbridge/amd/amdk8/raminit.c"
#include "northbridge/amd/amdk8/coherent_ht.c"
#include "sdram/generic_sdram.c"

 /* tyan does not want the default */
#include "resourcemap.c" 

#include "cpu/amd/dualcore/dualcore.c"

#define CK804_NUM 2
#define CK804B_BUSN 0x80
#define CK804_USE_NIC 1
#define CK804_USE_ACI 1

#include "southbridge/nvidia/ck804/ck804_early_setup_ss.h"

//set GPIO to input mode
#define CK804_MB_SETUP \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+ 5, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M9,GPIO6, PCIXB2_PRSNT1_L*/  \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+15, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M8,GPIO16, PCIXB2_PRSNT2_L*/ \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+44, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* P5,GPIO45, PCIXA_PRSNT1_L*/  \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+ 7, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* M5,GPIO8, PCIXA_PRSNT2_L*/   \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+16, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* K4,GPIO17, PCIXB_PRSNT1_L*/  \
                RES_PORT_IO_8, SYSCTRL_IO_BASE + 0xc0+45, ~(0xff),((0<<4)|(0<<2)|(0<<0)),/* P7,GPIO46, PCIXB_PRSNT2_L*/

#include "southbridge/nvidia/ck804/ck804_early_setup.c"

#include "cpu/amd/car/copy_and_run.c"

#include "cpu/amd/car/post_cache_as_ram.c"

#include "cpu/amd/model_fxx/init_cpus.c"


#if USE_FALLBACK_IMAGE == 1

#include "southbridge/nvidia/ck804/ck804_enable_rom.c"
#include "northbridge/amd/amdk8/early_ht.c"


static void sio_setup(void)
{

        unsigned value;
        uint32_t dword;
        uint8_t byte;

        
        pci_write_config32(PCI_DEV(0, CK804_DEVN_BASE+1, 0), 0xac, 0x047f0400);
        
        byte = pci_read_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0x7b);
        byte |= 0x20; 
        pci_write_config8(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0x7b, byte);
        
        dword = pci_read_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0xa0);
        dword |= (1<<29)|(1<<0);
        pci_write_config32(PCI_DEV(0, CK804_DEVN_BASE+1 , 0), 0xa0, dword);
        
#if  1  
        lpc47b397_enable_serial(SUPERIO_GPIO_DEV, SUPERIO_GPIO_IO_BASE);
                
        value =  lpc47b397_gpio_offset_in(SUPERIO_GPIO_IO_BASE, 0x77);
        value &= 0xbf; 
        lpc47b397_gpio_offset_out(SUPERIO_GPIO_IO_BASE, 0x77, value);
#endif

}

void failover_process(unsigned long bist, unsigned long cpu_init_detectedx)
{
        unsigned last_boot_normal_x = last_boot_normal();

        /* Is this a cpu only reset? or Is this a secondary cpu? */
        if ((cpu_init_detectedx) || (!boot_cpu())) {
                if (last_boot_normal_x) {
                        goto normal_image;
                } else {
                        goto fallback_image;
                }
        }

        /* Nothing special needs to be done to find bus 0 */
        /* Allow the HT devices to be found */

        enumerate_ht_chain();

        sio_setup();

        /* Setup the ck804 */
        ck804_enable_rom();

        /* Is this a deliberate reset by the bios */
        if (bios_reset_detected() && last_boot_normal_x) {
                goto normal_image;
        }
        /* This is the primary cpu how should I boot? */
        else if (do_normal_boot()) {
                goto normal_image;
        }
        else {
                goto fallback_image;
        }
 normal_image:
        __asm__ volatile ("jmp __normal_image"
                : /* outputs */
                : "a" (bist), "b" (cpu_init_detectedx) /* inputs */
                );

 fallback_image:
	;
}
#endif

void real_main(unsigned long bist, unsigned long cpu_init_detectedx);

void cache_as_ram_main(unsigned long bist, unsigned long cpu_init_detectedx)
{

#if USE_FALLBACK_IMAGE == 1
        failover_process(bist, cpu_init_detectedx);
#endif
        real_main(bist, cpu_init_detectedx);

}

void real_main(unsigned long bist, unsigned long cpu_init_detectedx)
{
	static const uint16_t spd_addr [] = {
			(0xa<<3)|0, (0xa<<3)|2, 0, 0,
			(0xa<<3)|1, (0xa<<3)|3, 0, 0,
#if CONFIG_MAX_PHYSICAL_CPUS > 1
			(0xa<<3)|4, (0xa<<3)|6, 0, 0,
			(0xa<<3)|5, (0xa<<3)|7, 0, 0,
#endif
	};

        int needs_reset;
	unsigned cpu_reset = 0;
        unsigned bsp_apicid = 0;

        struct mem_controller ctrl[8];
        unsigned nodes;

        if (bist == 0) {
                bsp_apicid = init_cpus(cpu_init_detectedx);
        }

	lpc47b397_enable_serial(SERIAL_DEV, TTYS0_BASE);
        uart_init();
        console_init();
	
	/* Halt if there was a built in self test failure */
	report_bist_failure(bist);

        setup_s2895_resource_map();

	needs_reset = setup_coherent_ht_domain();

#if CONFIG_LOGICAL_CPUS==1
        // It is said that we should start core1 after all core0 launched
	wait_all_core0_started();
        start_other_cores();
#endif

        wait_all_aps_started(bsp_apicid);

        needs_reset |= ht_setup_chains_x();

        needs_reset |= ck804_early_setup_x();

       	if (needs_reset) {
               	print_info("ht reset -\r\n");
               	soft_reset();
       	}

        allow_all_aps_stop(bsp_apicid);

        nodes = get_nodes();
        //It's the time to set ctrl now;
        fill_mem_ctrl(nodes, ctrl, spd_addr);

	enable_smbus();

	memreset_setup();
	sdram_initialize(nodes, ctrl);

	post_cache_as_ram(cpu_reset);
}
