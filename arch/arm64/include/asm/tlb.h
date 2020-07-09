#ifndef ASM_TLB_H
#define ASM_TLB_H

/* This macro creates a properly formatted VA operand for the TLBI */
#define __TLBI_VADDR(addr, asid)				\
	({							\
		unsigned long __ta = (addr) >> 12;		\
		__ta &= GENMASK_ULL(43, 0);			\
		__ta |= (unsigned long)(asid) << 48;		\
		__ta;						\
	})


void flush_tlb_kernel_range(unsigned long start, unsigned long end);

#endif /*ASM_TLB_H*/
