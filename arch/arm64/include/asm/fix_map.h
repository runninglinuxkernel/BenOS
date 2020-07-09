#ifndef ASM_FIX_MAP_H
#define ASM_FIX_MAP_H

#include <asm/pgtable.h>
#include <asm/pgtable_prot.h>
#include <asm/pgtable_hwdef.h>

enum fix_mapp_type {
	FIX_HOLE,
	FIX_RESERVE1,
	FIX_RESERVE2,
	FIX_PTE,
	FIX_PUD,
	FIX_PMD,
	FIX_PGD,
	FIX_PG_ALLOC,
	end_fix_map_type,
};

#define FIXADDR_SIZE	(end_fix_map_type << PAGE_SHIFT)
#define FIXADDR_START	(FIXADDR_TOP - FIXADDR_SIZE)

#define __fix_to_virt(x) (FIXADDR_TOP - ((x) << PAGE_SHIFT))
#define __virt_to_fix(x) ((FIXADDR_TOP - ((x)&PAGE_MASK)) >> PAGE_SHIFT)

#define set_fixmap_offset(idx, phys) \
	set_fix_mapping(idx, (unsigned long)phys, PAGE_KERNEL)

#define clear_fixmap(idx)			\
	set_fix_mapping(idx, 0, 0)

#define pgd_set_fixmap(addr) ((pgd_t *)set_fixmap_offset(FIX_PGD, addr))
#define pgd_clear_fixmap()  clear_fixmap(FIX_PGD)

#define pud_set_fixmap(addr) ((pud_t *)set_fixmap_offset(FIX_PUD, addr))
#define pud_clear_fixmap()  clear_fixmap(FIX_PUD)

#define pmd_set_fixmap(addr) ((pmd_t *)set_fixmap_offset(FIX_PMD, addr))
#define pmd_clear_fixmap()  clear_fixmap(FIX_PMD)

#define pte_set_fixmap(addr) ((pte_t *)set_fixmap_offset(FIX_PTE, addr))
#define pte_clear_fixmap()  clear_fixmap(FIX_PTE)

#define early_pg_set_fixmap(addr) (set_fixmap_offset(FIX_PG_ALLOC, addr))
#define early_pg_clear_fixmap()  clear_fixmap(FIX_PG_ALLOC)

unsigned long set_fix_mapping(enum fix_mapp_type idx,
		unsigned long phys, unsigned long prot);
int fix_map_init(void);

#endif /*ASM_FIX_MAP_H*/
