#ifndef	_MM_H
#define	_MM_H
#include <page.h>
#include <type.h>

#define TOTAL_MEMORY (SZ_1G)

#define LOW_MEMORY (2 * SECTION_SIZE)

/* 暂时使用1个4KB页面来当作内核栈*/
#define THREAD_SIZE  (1 * PAGE_SIZE)
/* sp必须16字节对齐，因为arm扩展的_int128类型是16字节 */
#define THREAD_START_SP  (THREAD_SIZE - 16)

#ifndef __ASSEMBLER__
void memzero(unsigned long src, unsigned long n);
#endif

#endif  /*_MM_H */
