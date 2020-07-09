#ifndef	ASM_MM_H
#define	ASM_MM_H

#include <mach/base.h>
#include <type.h>

#define TOTAL_MEMORY (SZ_1G)

#define PAGE_SHIFT 12

#define PAGE_SIZE (1 << PAGE_SHIFT)

#define PAGE_MASK (~(PAGE_SIZE-1))

/* 暂时使用1个4KB页面来当作内核栈*/
#define THREAD_SIZE  (1 * PAGE_SIZE)
/* sp必须16字节对齐，因为arm扩展的_int128类型是16字节 */
#define THREAD_START_SP  (THREAD_SIZE - 16)

#define ARCH_PFN_OFFSET (ARCH_PHYS_OFFSET >> PAGE_SHIFT)

/* CONFIG_ARM64_VA_BITS = 48*/
#define CONFIG_ARM64_VA_BITS 48
#define VA_BITS	 (CONFIG_ARM64_VA_BITS)

/* Memory layout */

/* 内核空间起始地址: 0xffff_0000_0000_0000*/
#define VA_START (UL(0xffffffffffffffff) - \
	(UL(1) << VA_BITS) + 1)

/* 线性映射地址: 0xffff_8000_0000_0000*/
#define PAGE_OFFSET (UL(0xffffffffffffffff) - \
	(UL(1) << (VA_BITS - 1)) + 1)

/* 内核image链接起始地址 */
#define KIMAGE_VADDR	(VA_START + 0x10000000)
#define TEXT_OFFSET 0x80000

/* fixmap区间虚拟地址*/
#define FIXADDR_TOP (PAGE_OFFSET - SZ_16M)

#define PHYS_OFFSET ARCH_PHYS_OFFSET

/*
 * 线性映射的虚拟地址 -> 物理地址
 *
 * 线性地址 减去 PAGE_OFFSET 得到物理地址
 */
#define __linear_to_phys(addr) (((unsigned long)(addr) & ~PAGE_OFFSET) \
					+ PHYS_OFFSET)

/* 内核符号链接地址（虚拟地址） -> 物理地址
 *
 * 链接地址减去 KIMAGE_VADDR 就得到物理地址
 */
#define __kimg_to_phys(addr)	(((unsigned long)(addr)) - KIMAGE_VADDR)

/*
 * 根据虚拟地址来判断:
 * 是线性映射虚拟地址 还是 内核符号链接地址（虚拟地址）
 */
#define __is_linear_address(addr)	(!!((addr) & BIT(VA_BITS - 1)))

#define __pa(x)  ({\
		unsigned long x1 = (unsigned long)(x);\
		__is_linear_address(x1) ? __linear_to_phys(x1) :\
		__kimg_to_phys(x1);\
})

#define __pa_symbol(x)  __kimg_to_phys(x)

#define __phys_to_virt(x) ((unsigned long)((x) - PHYS_OFFSET) | PAGE_OFFSET)
#define __phys_to_kimg(x) ((unsigned long)((x) + KIMAGE_VADDR))

#define __va(x) ((void *)__phys_to_virt((unsigned long)x))

/*
 * Memory types available.
 */
#define MT_DEVICE_nGnRnE	0
#define MT_DEVICE_nGnRE		1
#define MT_DEVICE_GRE		2
#define MT_NORMAL_NC		3
#define MT_NORMAL		4
#define MT_NORMAL_WT		5

#define MAIR(attr, mt)	((attr) << ((mt) * 8))

#ifndef __ASSEMBLER__
void memzero(unsigned long src, unsigned long n);
unsigned long bootmem_get_start_ddr(void);
unsigned long bootmem_get_end_ddr(void);
void memblock_init(void);
void bootmem_init(void);
void mem_init(void);
void paging_init(void);
void dump_pgtable(void);

#endif

#endif  /*ASM_MM_H */
