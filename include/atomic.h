#ifndef ATOMIC_H
#define ATOMIC_H

#include <asm/types.h>
#include <asm/atomic.h>

#ifndef _HAS_ASM_SET_BIT
static inline void set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr);

	*p  |= mask;
}
#endif

#ifndef _HAS_ASM_CLEAR_BIT
static inline void clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr);

	*p &= ~mask;
}
#endif

static inline int test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (*addr >> (nr & (BITS_PER_LONG-1)));
}

#endif /*ATOMIC_H*/
