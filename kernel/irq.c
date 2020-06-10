#include <irq.h>
#include <sched.h>
#include <printk.h>
#include <memory.h>
#include <string.h>

struct irq_desc irq_desc[NR_IRQS];

struct irq_desc *irq_to_desc(unsigned int irq)
{
	return (irq < NR_IRQS) ? irq_desc + irq : NULL;
}

int irq_alloc_descs(unsigned int start, unsigned int count)
{
	int i;

	for (i = 0; i < count; i++) {
		struct irq_desc *desc = irq_to_desc(start + i);

		desc->refcount++;
	}

	return start;
}

static int irq_domain_map_irq(struct irq_domain *domain,
		unsigned int virq, unsigned int hwirq)
{
	int ret;
	struct irq_desc *desc = irq_to_desc(virq);

	desc->hwirq = hwirq;
	desc->domain = domain;

	if (domain->ops->map) {
		ret = domain->ops->map(domain, virq, hwirq);
		if (ret) {
			printk("%s didn't like hwirq %d to virq %d mapping\n",
					domain->name, hwirq, virq);
			return ret;
		}
	}

	return 0;
}

unsigned int irq_find_mapping(struct irq_domain *domain, unsigned int hwirq)
{
	/* FIXME: hwirq = virq now */
	return hwirq;
}

void irq_enable(struct irq_desc *desc)
{
	if (desc->chip->irq_enable)
		desc->chip->irq_enable(desc);
	else
		desc->chip->irq_unmask(desc);
}

void irq_disable(struct irq_desc *desc)
{
	if (desc->chip->irq_disable)
		desc->chip->irq_disable(desc);
	else
		desc->chip->irq_mask(desc);
}

int request_irq(unsigned int irq, irq_handler_t handler,
		const char *name, void *param)
{
	struct irq_desc *desc;

	desc = irq_to_desc(irq);
	if (!desc)
		return -1;
	if (!handler)
		return -1;

	desc->handler = handler;
	desc->param = param;
	desc->name = name;

	irq_enable(desc);

	return 0;
}

void free_irq(unsigned int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	desc->handler = NULL;
	desc->param = NULL;
}

int generic_handle_irq(unsigned int virq)
{
	struct irq_desc *desc = irq_to_desc(virq);
	int ret;

	if (!desc || !desc->handler)
		return -1;

	ret = desc->handler(virq, desc->param);

	/* send eoi */
	if (desc->chip->irq_eoi)
		desc->chip->irq_eoi(desc);

	return ret;
}

int handle_domain_irq(struct irq_domain *domain, unsigned int hwirq)
{
	unsigned int virq;

	virq = irq_find_mapping(domain, hwirq);

	generic_handle_irq(virq);

	return 0;
}

struct irq_domain *irq_domain_add(unsigned int virq, unsigned int hwirq,
		unsigned int count, const struct irq_domain_ops *ops,
		void *host_data)
{

	struct irq_domain *domain;
	int i;

	/* FIXME: using kmalloc later*/
	domain = (struct irq_domain *)get_free_page();
	if (!domain)
		return NULL;

	memset(domain, 0, sizeof(*domain));

	domain->ops = ops;
	domain->host_data = host_data;
	domain->hwirq_max = hwirq + count;

	for (i = 0; i < count; i++)
		irq_domain_map_irq(domain, virq + i, hwirq + i);

	return domain;
}

int irq_domain_set_hwirq_chip(struct irq_domain *domain, unsigned int virq,
		unsigned int hwirq, struct irq_chip *chip, void *chip_data)
{
	struct irq_desc *desc = irq_to_desc(virq);

	if (!desc)
		return -1;

	desc->hwirq = hwirq;
	desc->chip = chip;
	desc->chip_data = chip_data;

	return 0;
}

void (*arch_irq_handle)() = NULL;

void set_handle_irq(void (*handle_irq)())
{
	if (arch_irq_handle)
		return;

	arch_irq_handle = handle_irq;
}

void irq_handle(void)
{
	__irq_enter();
	arch_irq_handle();
	__irq_exit();
}
