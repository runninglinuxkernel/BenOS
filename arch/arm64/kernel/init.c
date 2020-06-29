#include <asm/irq.h>
#include <asm/mm.h>

void setup_arch(void)
{
	memblock_init();
	bootmem_init();
	mem_init();

	arch_irq_init();
}
