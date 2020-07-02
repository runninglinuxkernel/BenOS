#ifndef ASM_ATOMIC_H
#define ASM_ATOMIC_H

#include <atomic.h>

static inline void atomic64_orr(long nr, atomic64_t *v)
{

	unsigned long tmp;
	long result;

	asm volatile (
		" prfm pstl1strm, %[counter]\n"
		"1: ldxr %[result], %[counter]\n"
		" orr %[result], %[result], %[nr]\n"
		" stxr %w[tmp], %[result], %[counter]\n"
		" cbnz %w[tmp], 1b"
		: [result] "=&r" (result), [tmp] "=&r" (tmp),
			[counter] "+Q" (v->counter)
		: [nr] "Ir" (nr)
		: "memory");
}

#define _HAS_ASM_SET_BIT
static inline void set_bit(unsigned int nr, volatile unsigned long *p)
{
	atomic64_orr(BIT_MASK(nr), (atomic64_t *)p);
}

static inline void atomic64_bic(long nr, atomic64_t *v)
{

	unsigned long tmp;
	long result;

	asm volatile (
		" prfm pstl1strm, %[counter]\n"
		"1: ldxr %[result], %[counter]\n"
		" bic %[result], %[result], %[nr]\n"
		" stxr %w[tmp], %[result], %[counter]\n"
		" cbnz %w[tmp], 1b"
		: [result] "=&r" (result), [tmp] "=&r" (tmp),
			[counter] "+Q" (v->counter)
		: [nr] "Ir" (nr)
		: "memory");
}

#define _HAS_ASM_CLEAR_BIT
static inline void clear_bit(unsigned int nr, volatile unsigned long *p)
{
	atomic64_bic(BIT_MASK(nr), (atomic64_t *)p);
}

#endif /* ASM_ATOMIC_H*/
