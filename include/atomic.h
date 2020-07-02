#ifndef ATOMIC_H
#define ATOMIC_H

#include <asm/types.h>

#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))

typedef struct {
	int counter;
} atomic_t;

typedef struct {
	long counter;
} atomic64_t;

static inline void set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr);

	*p  |= mask;
}

static inline void clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr);

	*p &= ~mask;
}

static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (*addr >> (nr & (BITS_PER_LONG-1)));
}

#endif /*ATOMIC_H*/
