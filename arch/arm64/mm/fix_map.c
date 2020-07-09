#include <asm/pgtable.h>
#include <asm/fix_map.h>
#include <asm/tlb.h>

pte_t fix_map_pte[PTRS_PER_PTE] __attribute__((__section__(".bss.page_aligned")));
pmd_t fix_map_pmd[PTRS_PER_PMD] __attribute__((__section__(".bss.page_aligned")));
pud_t fix_map_pud[PTRS_PER_PUD] __attribute__((__section__(".bss.page_aligned")));

static inline pte_t * fixmap_pte(unsigned long addr)
{
	return &fix_map_pte[pte_index(addr)];
}

unsigned long set_fix_mapping(enum fix_mapp_type idx,
		unsigned long phys, unsigned long prot)
{
	unsigned long vaddr = __fix_to_virt(idx);
	pte_t *ptep;

	if (idx > end_fix_map_type)
		return 0;

	ptep = fixmap_pte(vaddr);

	if (prot) {
		set_pte(ptep, pfn_pte(phys >> PAGE_SHIFT, prot));
	} else {
		set_pte(ptep, __pte(0));
		flush_tlb_kernel_range(vaddr, vaddr+PAGE_SIZE);
	}

	vaddr += phys & (PAGE_SIZE - 1);

	return vaddr;
}

static pmd_t * fixmap_pmd(pud_t *pudp, unsigned long addr)
{
	pmd_t *pmd = pmd_offset_phys(pudp, addr);

	return (pmd_t *)__phys_to_kimg((unsigned long)pmd);
}

static pud_t * fixmap_pud(pgd_t *pgdp, unsigned long addr)
{
	pud_t *pudp = pud_offset_phys(pgdp, addr);

	return (pud_t *)__phys_to_kimg((unsigned long)pudp);
}

int fix_map_init(void)
{
	pgd_t *pgdp, pgd;
	pud_t *pudp, pud;
	pmd_t *pmdp, pmd;
	unsigned long addr = FIXADDR_START;
	
	pgdp = pgd_offset_raw((pgd_t *)init_pg_dir, addr);

	pgd = *pgdp;
	if (pgd_none(pgd))
		set_pgd(pgdp, __pgd(
				(unsigned long)__pa_symbol((unsigned long)fix_map_pud)
					| PUD_TYPE_TABLE));

	pudp = fixmap_pud(pgdp, addr);
	pud = *pudp;
	if (pud_none(pud))
		set_pud(pudp, __pud((unsigned long)__pa_symbol((unsigned long)fix_map_pmd)
					| PUD_TYPE_TABLE));
	
	pmdp = fixmap_pmd(pudp, addr);
	pmd = *pmdp;
	if (pmd_none(pmd)) 
		set_pmd(pmdp, __pmd((unsigned long)__pa_symbol((unsigned long)fix_map_pte)
					| PMD_TYPE_TABLE));

	return 0;
}
