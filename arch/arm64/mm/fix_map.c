#include <asm/pgtable.h>
#include <asm/fix_map.h>
#include <asm/tlb.h>

/*
 * fix_map机制：
 * 目的：
 * 在内存还没有建立线性映射之前，memblock机制分配的内存
 * 是不能正确被CPU访问的,因为此时CPU已经打开了MMU，
 * memblock分配的物理页面还没有建立映射关系，即等不到
 * 虚拟地址。
 *
 * 在建立页表的过程中，就需要使用memblock机制来分配内存,
 * 用作页表本身，例如PUD,PMD,PTE页表本身就是使用一个page
 * 来存储页表项。 memblock分配的page可能存在乱数据，这对
 * 页表本身是一个坏事情，我们需要把这个page清0。此时，
 * CPU需要得到这个page的虚拟
 * 地址才能调用memset()来清0。
 *
 * 解决办法： 采用临时映射。
 * 在内核空间某个地址处（这个地址必须和2MB对齐）创建一
 * 个fix map region使用的虚拟地址空间，使用预先映射的方
 * 式来建立PUD, PMD页表。当需要为一个物理页面建立映射
 * 之后，从fix map region中取一个虚拟页面填充对应pte页
 * 表项，即完成了fix map映射，然后返回这个虚拟地址即可。
 */

/*
 * 预先分配3个page，用于fix map机制的PUD, PMD, PTE页表
 * 链接到bss段，这样它们的内容全是0
 */
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
	/*
	 * 注意:
	 * 清空页表项时，需要flush TLB，否则下次建立
	 * 映射时，CPU还访问了之前映射的旧数据,
	 * 导致程序出错。
	 */
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
