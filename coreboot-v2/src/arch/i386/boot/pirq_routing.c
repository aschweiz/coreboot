#include <console/console.h>
#include <arch/pirq_routing.h>
#include <string.h>

#if (DEBUG==1 && HAVE_PIRQ_TABLE==1)
static void check_pirq_routing_table(struct irq_routing_table *rt)
{
	uint8_t *addr = (uint8_t *)rt;
	uint8_t sum=0;
	int i;

	printk_info("Checking IRQ routing table consistency...\n");

#if defined(IRQ_SLOT_COUNT)
	if (sizeof(struct irq_routing_table) != rt->size) {
		printk_warning("Inconsistent IRQ routing table size (0x%x/0x%x)\n",
			       sizeof(struct irq_routing_table),
			       rt->size
			);
		rt->size=sizeof(struct irq_routing_table);
	}
#endif

	for (i = 0; i < rt->size; i++)
		sum += addr[i];

	printk_debug("%s() - irq_routing_table located at: 0x%p\n",
		     __FUNCTION__, addr);

	
	sum = rt->checksum - sum;

	if (sum != rt->checksum) {
		printk_warning("%s:%6d:%s() - "
			       "checksum is: 0x%02x but should be: 0x%02x\n",
			       __FILE__, __LINE__, __FUNCTION__, rt->checksum, sum);
		rt->checksum = sum;
	}

	if (rt->signature != PIRQ_SIGNATURE || rt->version != PIRQ_VERSION ||
	    rt->size % 16 ) {
		printk_warning("%s:%6d:%s() - "
			       "Interrupt Routing Table not valid\n",
			       __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	sum = 0;
	for (i=0; i<rt->size; i++)
		sum += addr[i];

	if (sum) {
		printk_warning("%s:%6d:%s() - "
			       "checksum error in irq routing table\n",
			       __FILE__, __LINE__, __FUNCTION__);
	}

	printk_info("done.\n");
}

static int verify_copy_pirq_routing_table(unsigned long addr)
{
	int i;
	uint8_t *rt_orig, *rt_curr;

	rt_curr = (uint8_t*)addr;
	rt_orig = (uint8_t*)&intel_irq_routing_table;
	printk_info("Verifing copy of IRQ routing tables at 0x%x...", addr);
	for (i = 0; i < intel_irq_routing_table.size; i++) {
		if (*(rt_curr + i) != *(rt_orig + i)) {
			printk_info("failed\n");
			return -1;
		}
	}
	printk_info("done\n");
	
	check_pirq_routing_table((struct irq_routing_table *)addr);
	
	return 0;
}
#else
#define verify_copy_pirq_routing_table(addr)
#endif

#if HAVE_PIRQ_TABLE==1
unsigned long copy_pirq_routing_table(unsigned long addr)
{
	/* Align the table to be 16 byte aligned. */
	addr += 15;
	addr &= ~15;

	/* This table must be betweeen 0xf0000 & 0x100000 */
	printk_info("Copying IRQ routing tables to 0x%x...", addr);
	memcpy((void *)addr, &intel_irq_routing_table, intel_irq_routing_table.size);
	printk_info("done.\n");
	verify_copy_pirq_routing_table(addr);
	return addr + intel_irq_routing_table.size;
}
#endif