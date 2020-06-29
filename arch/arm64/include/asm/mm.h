#ifndef	_MM_H
#define	_MM_H
#include <mach/base.h>

#define TOTAL_MEMORY (SZ_1G)

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define PAGE_MASK (~(PAGE_SIZE-1))

/* 暂时使用1个4KB页面来当作内核栈*/
#define THREAD_SIZE  (1 * PAGE_SIZE)
#define THREAD_START_SP  (THREAD_SIZE - 8)

#define ARCH_PFN_OFFSET (ARCH_PHYS_OFFSET >> PAGE_SHIFT)

#ifndef __ASSEMBLER__
void memzero(unsigned long src, unsigned long n);
void memblock_init(void);
void bootmem_init(void);
void mem_init(void);
#endif

#endif  /*_MM_H */
