#include <mach/irq.h>
#include <asm/io.h>
#include <printk.h>
#include <arm-gic.h>
#include <pi-irq.h>

void arch_irq_init(void)
{
#ifdef CONFIG_ARM_GICV2
	gic_init(0, GIC_V2_DISTRIBUTOR_BASE, GIC_V2_CPU_INTERFACE_BASE);
#else
	pi_irq_init(ARMC_BASE, ARM_LOCAL_BASE);
#endif
}
