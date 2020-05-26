#include <asm/mm.h>

#define NR_PAGES (TOTAL_MEMORY / PAGE_SIZE)

static unsigned short mem_map[NR_PAGES] = {0,};

void mem_init(unsigned long start_mem, unsigned long end_mem)
{

}

unsigned long get_free_page(void)
{
	int i;

	for (i = 0; i < NR_PAGES; i++) {
		if (mem_map[i] == 0) {
			mem_map[i] = 1;
			return LOW_MEMORY + i * PAGE_SIZE;
		}
	}
	return 0;
}

void free_page(unsigned long p)
{
	mem_map[(p - LOW_MEMORY)/PAGE_SIZE] = 0;
}
