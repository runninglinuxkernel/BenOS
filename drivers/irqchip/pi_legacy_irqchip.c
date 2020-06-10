#include <type.h>
#include <irq.h>
#include <printk.h>
#include <mach/irq.h>
#include <asm/io.h>
#include <bitops.h>

/*
 * Legacy interrupt for PI
 *
 *  0 ~ 63 :  VideoCore interrupts (ARMC)
 *              PI4: IRQ0_SET_EN_0, IRQ0_SET_EN_1
 *              PI3: IRQ_PENDING1, IRQ_PENDING2
 * 64 ~ 85 :  ARMC interrupts
 *              PI4: IRQ0_SET_EN_2
 *              PI3: IRQ_BASE_PENDING
 *
 * 86 ~ 118:  ARM Local interrupts (TIMER_CNTRL0)
 */

#define NR_IRQS_PI 128

struct pi_chip_data {
	unsigned long armc_base;
	unsigned long arm_local_base;
	struct irq_domain *domain;
	struct irq_chip *chip;
	unsigned int irqs;
};

#define pi_armc_base(d) ((d)->armc_base)
#define pi_arml_base(d) ((d)->arm_local_base)

static struct pi_chip_data pi_data;

static unsigned long pi_get_armc_base(struct irq_desc *d)
{
	struct pi_chip_data *pi = irq_get_chip_data(d);

	return pi_armc_base(pi);
}

static unsigned long pi_get_arml_base(struct irq_desc *d)
{
	struct pi_chip_data *pi = irq_get_chip_data(d);

	return pi_arml_base(pi);
}

static void pi_irq_mask_irq(struct irq_desc *d)
{
	unsigned int hwirq = d->hwirq;
	unsigned int value;

	if (hwirq < 32)
		writel(1 << hwirq, pi_get_armc_base(d) + PI_IRQ_BANK0_DISABLE);
	else if (hwirq < 64)
		writel(1 << (hwirq % 32), pi_get_armc_base(d) +
				PI_IRQ_BANK1_DISABLE);
	else if (hwirq < 86)
		writel(1 << (hwirq - 64), pi_get_armc_base(d) +
				PI_IRQ_BANK2_DISABLE);
	else {
		value = readl(pi_get_arml_base(d) + TIMER_CNTRL0);
		value &= ~(1 << (hwirq - 86));
		writel(value, pi_get_arml_base(d) + TIMER_CNTRL0);
	}
}

static void pi_irq_unmask_irq(struct irq_desc *d)
{
	unsigned int hwirq = d->hwirq;

	if (hwirq < 32)
		writel(1 << hwirq, pi_get_armc_base(d) + PI_IRQ_BANK0_ENABLE);
	else if (hwirq < 64)
		writel(1 << (hwirq - 32), pi_get_armc_base(d) +
				PI_IRQ_BANK1_ENABLE);
	else if (hwirq < 86)
		writel(1 << (hwirq - 64), pi_get_armc_base(d) +
				PI_IRQ_BANK2_ENABLE);
	else
		writel(1 << (hwirq - 86), pi_get_arml_base(d) + TIMER_CNTRL0);
}

static struct irq_chip pi_irq_chip = {
	.name = "legacy irq for pi",
	.irq_mask = pi_irq_mask_irq,
	.irq_unmask = pi_irq_unmask_irq,
};

static int pi_irq_map(struct irq_domain *d, unsigned int virq,
		unsigned int hwirq)
{
	irq_domain_set_hwirq_chip(d, virq, hwirq, &pi_irq_chip, d->host_data);

	return 0;
}

static const struct irq_domain_ops pi_irq_domain_ops = {
	.map = pi_irq_map,
};

static void pi_irq_handle_irq(void)
{
	unsigned int pending;
	struct pi_chip_data *pi = &pi_data;
	unsigned int value;
	unsigned int irq = 0;

	/* check ARM LOCAL irq first */
	value = readl(pi_arml_base(pi) + ARM_LOCAL_IRQ_SOURCE0);
	if (value) {
		irq = ffs(value) - 1;
		irq += 86;
	} else {
		pending = readl(pi_armc_base(pi) + PI_IRQ_BASE_PENDING);
		switch (pending) {
		case BAND0_STATUS:
			value = readl(pi_armc_base(pi) + PI_IRQ_BANK0_PENDING);
			irq = ffs(value) - 1;
			break;
		case BAND1_STATUS:
			value = readl(pi_armc_base(pi) + PI_IRQ_BANK1_PENDING);
			irq = ffs(value) - 1;
			irq += 32;
			break;
		default:
			irq = ffs(pending) - 1;
			irq += 64;
			break;
		}
	}

	if (irq)
		handle_domain_irq(pi->domain, irq);
}

int pi_irq_init(unsigned long armc_base, unsigned long arm_local_base)
{
	struct pi_chip_data *pi;
	unsigned int virq_base;

	pi = &pi_data;
	pi->armc_base = armc_base;
	pi->arm_local_base = arm_local_base;
	pi->chip = &pi_irq_chip;
	pi->irqs = NR_IRQS_PI;

	virq_base = irq_alloc_descs(0, pi->irqs);

	pi->domain = irq_domain_add(virq_base, 0, pi->irqs,
			&pi_irq_domain_ops, pi);
	if (!pi->domain)
		return -1;

	printk("%s: armc_base:0x%x, arm_local:0x%x, irqs:%d, virq:%d\n",
			__func__, armc_base, arm_local_base,
			pi->irqs, virq_base);

	set_handle_irq(pi_irq_handle_irq);

	return 0;
}
