#ifndef IRQ_H
#define IRQ_H

#include <asm/irq.h>

#define raw_local_irq_disable()	arch_local_irq_disable()
#define raw_local_irq_enable()	arch_local_irq_enable()

#define NR_IRQS 512

struct irq_desc;
struct irq_domain;

/**
 * struct irq_chip - hardware interrupt chip descripter
 */
struct irq_chip {
	const char *name;
	void (*irq_enable)(struct irq_desc *d);
	void (*irq_disable)(struct irq_desc *d);
	void (*irq_ack)(struct irq_desc *d);
	void (*irq_mask)(struct irq_desc *d);
	void (*irq_unmask)(struct irq_desc *d);
	void (*irq_eoi)(struct irq_desc *d);
};

typedef int (*irq_handler_t)(int, void *);

/**
 * struct irq_desc - interrupt descripter
 */
struct irq_desc {
	const char *name;
	unsigned int virq;
	unsigned int hwirq;
	struct irq_chip *chip;
	struct irq_domain *domain;
	void *chip_data;
	unsigned int refcount;

	irq_handler_t handler;
	void *param;
};

struct irq_domain_ops {
	int (*map)(struct irq_domain *d, unsigned int virq, unsigned int hwirq);
	void (*unmap)(struct irq_domain *d, unsigned int virq);
};

/**
 * struct irq_domain - Hardware interrupt number translation object
 */
struct irq_domain {
	const char *name;
	const struct irq_domain_ops *ops;
	unsigned int hwirq_max;
	void *host_data;
};

static inline void *irq_get_chip_data(struct irq_desc *d)
{
	return d->chip_data;
}

int irq_alloc_descs(unsigned int start, unsigned int count);
struct irq_desc *irq_to_desc(unsigned int irq);
void irq_enable(struct irq_desc *desc);
void irq_disable(struct irq_desc *desc);
int request_irq(unsigned int irq, irq_handler_t handler,
		const char *name, void *param);
void free_irq(unsigned int irq);
struct irq_domain *irq_domain_add(unsigned int virq, unsigned int hwirq,
		unsigned int count, const struct irq_domain_ops *ops,
		void *host_data);
int irq_domain_set_hwirq_chip(struct irq_domain *domain, unsigned int virq,
		unsigned int hwirq, struct irq_chip *chip, void *chip_data);
void set_handle_irq(void (*handle_irq)());
int generic_handle_irq(unsigned int virq);
unsigned int irq_find_mapping(struct irq_domain *domain, unsigned int hwirq);
int handle_domain_irq(struct irq_domain *domain, unsigned int hwirq);

#endif /*IRQ_H*/
