#ifndef	ASM_MM_H
#define	ASM_MM_H

#include <mach/base.h>

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
