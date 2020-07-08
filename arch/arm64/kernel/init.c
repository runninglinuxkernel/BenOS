#include <asm/irq.h>
#include <asm/mm.h>
#include <printk.h>
#include <asm/uart.h>

void setup_arch(void)
{
	memblock_init();
	paging_init();

	uart_init();
	init_printk_done();

	bootmem_init();
	mem_init();

	arch_irq_init();
}
