#include <asm/pgtable.h>
#include <asm/pgtable_prot.h>
#include <asm/pgtable_hwdef.h>
#include <asm/sysregs.h>
#include <page.h>
#include <asm/barrier.h>
#include <memblock.h>
#include <printk.h>
#include <string.h>
#include <asm/fix_map.h>

#define NO_BLOCK_MAPPINGS BIT(0)
#define NO_CONT_MAPPINGS BIT(1)

extern char _text_boot[], _etext_boot[];
extern char _text[], _etext[];
extern char _rodata[], _erodata[];
extern char _data[], _edata[];
extern char _bss[], _ebss[];
extern char  _end[];

static void alloc_init_pte(pmd_t *pmdp, unsigned long addr,
		unsigned long end, unsigned long phys,
		unsigned long prot,
		unsigned long (*alloc_pgtable)(void),
		unsigned long flags)
{
	pmd_t pmd = *pmdp;
	pte_t *ptep;

	if (pmd_none(pmd)) {
		unsigned long pte_phys;

		pte_phys = alloc_pgtable();
		set_pmd(pmdp, __pmd(pte_phys | PMD_TYPE_TABLE));
		pmd = *pmdp;
	}

	ptep = pte_set_fixmap_offset(pmdp, addr);
	do {
		set_pte(ptep, pfn_pte(phys >> PAGE_SHIFT, prot));
		phys += PAGE_SIZE;
	} while (ptep++, addr += PAGE_SIZE, addr != end);

	pte_clear_fixmap();
}

void pmd_set_section(pmd_t *pmdp, unsigned long phys,
		unsigned long prot)
{
	unsigned long sect_prot = PMD_TYPE_SECT | mk_sect_prot(prot);

	pmd_t new_pmd = pfn_pmd(phys >> PMD_SHIFT, sect_prot);

	set_pmd(pmdp, new_pmd);
}

static void alloc_init_pmd(pud_t *pudp, unsigned long addr,
		unsigned long end, unsigned long phys,
		unsigned long prot,
		unsigned long (*alloc_pgtable)(void),
		unsigned long flags)
{
	pud_t pud = *pudp;
	pmd_t *pmdp;
	unsigned long next;

	if (pud_none(pud)) {
		unsigned long pmd_phys;

		pmd_phys = alloc_pgtable();
		set_pud(pudp, __pud(pmd_phys | PUD_TYPE_TABLE));
		pud = *pudp;
	}

	pmdp = pmd_set_fixmap_offset(pudp, addr);
	do {
		next = pmd_addr_end(addr, end);

		if (((addr | next | phys) & ~SECTION_MASK) == 0 &&
				(flags & NO_BLOCK_MAPPINGS) == 0)
			pmd_set_section(pmdp, phys, prot);
		else
			alloc_init_pte(pmdp, addr, next, phys,
					prot,  alloc_pgtable, flags);

		phys += next - addr;
	} while (pmdp++, addr = next, addr != end);

	pmd_clear_fixmap();
}

static void alloc_init_pud(pgd_t *pgdp, unsigned long addr,
		unsigned long end, unsigned long phys,
		unsigned long prot,
		unsigned long (*alloc_pgtable)(void),
		unsigned long flags)
{
	pgd_t pgd = *pgdp;
	pud_t *pudp;
	unsigned long next;

	if (pgd_none(pgd)) {
		unsigned long pud_phys;

		pud_phys = alloc_pgtable();

		set_pgd(pgdp, __pgd(pud_phys | PUD_TYPE_TABLE));
		pgd = *pgdp;
	}

	pudp = pud_set_fixmap_offset(pgdp, addr);
	do {
		next = pud_addr_end(addr, end);
		alloc_init_pmd(pudp, addr, next, phys,
				prot, alloc_pgtable, flags);
		phys += next - end;

	} while (pudp++, addr = next, addr != end);

	pud_clear_fixmap();
}

static void __create_pgd_mapping(pgd_t *pgdir, unsigned long phys,
		unsigned long virt, unsigned long size,
		unsigned long prot,
		unsigned long (*alloc_pgtable)(void),
		unsigned long flags)
{
	pgd_t *pgdp = pgd_offset_raw(pgdir, virt);
	unsigned long addr, end, next;

	phys &= PAGE_MASK;
	addr = virt & PAGE_MASK;
	end = PAGE_ALIGN(virt + size);

	do {
		next = pgd_addr_end(addr, end);
		alloc_init_pud(pgdp, addr, next, phys,
				prot, alloc_pgtable, flags);
		phys += next - addr;
	} while (pgdp++, addr = next, addr != end);
}

static unsigned long early_pgtable_alloc(void)
{
	void *phys;
	void *vaddr;

	phys = memblock_phys_alloc(PAGE_SIZE);
	vaddr = (void *)early_pg_set_fixmap((unsigned long)phys);

	memset(vaddr, 0, PAGE_SIZE);

	early_pg_clear_fixmap();

	return (unsigned long)phys;
}

static void create_kernel_mapping(pgd_t *pgdp)
{
	unsigned long phy_start, vaddr, size, vend;

	/*map text*/
	vaddr = (unsigned long)_text_boot;
	vend = (unsigned long)_etext;
	phy_start = __pa_symbol(vaddr);
	size = vend - vaddr;

	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL_ROX,
			early_pgtable_alloc,
			0);

	/*map ro data*/
	vaddr = (unsigned long)_rodata;
	vend = (unsigned long)_erodata;
	phy_start = __pa_symbol(vaddr);
	size = vend - vaddr;
	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL_RO,
			early_pgtable_alloc,
			0);

	/*map data*/
	vaddr = (unsigned long)_data;
	vend = (unsigned long)_end;
	phy_start = __pa_symbol(vaddr);
	size = vend - vaddr;
	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL,
			early_pgtable_alloc,
			0);
}

static void create_mem_mapping(pgd_t *pgdp)
{
	unsigned long phy_start, vaddr, size;
	unsigned long phy_end;
	unsigned long kernel_start = __pa_symbol(_text_boot);
	unsigned long kernel_end = __pa_symbol(_erodata);

	/*1. map 0 address to kernel text section*/
	phy_start = bootmem_get_start_ddr();
	vaddr = __phys_to_virt(phy_start);
	size = kernel_start - phy_start;
	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL,
			early_pgtable_alloc,
			0);

	/*2. map kernel text section with RO and no exec*/
	vaddr = __phys_to_virt(kernel_start);
	phy_start = kernel_start;
	size = kernel_end - kernel_start;
	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL_RO,
			early_pgtable_alloc,
			0);

	/*3. map other memory*/
	phy_end = bootmem_get_end_ddr();
	phy_start = kernel_end;
	size = phy_end - phy_start;
	vaddr = __phys_to_virt(phy_start);
	__create_pgd_mapping(pgdp, phy_start, vaddr,
			size, PAGE_KERNEL,
			early_pgtable_alloc,
			0);
}

static void create_mmio_mapping(pgd_t *pgdp)
{
	__create_pgd_mapping(pgdp, PBASE, VA_START + PBASE,
			DEVICE_SIZE, PROT_DEVICE_nGnRnE,
			early_pgtable_alloc,
			0);
}

static void map_kernel_mem(pgd_t *pgdp)
{
	create_kernel_mapping(pgdp);
	create_mem_mapping(pgdp);
}

static void map_mmio(pgd_t *pgdp)
{
	create_mmio_mapping(pgdp);
}

void paging_init(void)
{
	fix_map_init();

	pgd_t *pgdp = pgd_set_fixmap(__pa_symbol(init_pg_dir));

	map_kernel_mem(pgdp);
	map_mmio(pgdp);

	pgd_clear_fixmap();
}
