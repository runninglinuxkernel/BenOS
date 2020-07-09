#include <asm/tlb.h>
#include <asm/barrier.h>
#include <page.h>

#define TLBI(op, arg) asm (   \
	 "tlbi " #op " ,%0\n" \
	 "dsb ish\n"  \
	 "tlbi " #op " ,%0\n" \
	 :: "r" (arg)) 


void flush_tlb_kernel_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start = __TLBI_VADDR(start, 0);
	end = __TLBI_VADDR(end, 0);	

	dsb(ishst);
	for (addr = start; addr < end; addr += 1 << (PAGE_SHIFT - 12))
		TLBI(vaale1is, addr);
	dsb(ish);
	isb();
}
