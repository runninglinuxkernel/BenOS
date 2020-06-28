
enum memblock_flags {
	MEMBLOCK_FREE = 0,
	MEMBLOCK_RESERVE = 1,
};

struct memblock_region {
	unsigned long base;
	unsigned long size;
	enum memblock_flags flags;
	unsigned int allocated;
	struct memblock_region *prev;
	struct memblock_region *next;
};

struct memblock {
	struct memblock_region head;
	unsigned int num_regions;
	unsigned long total_size;
};

#define MAX_MEMBLOCK_REGIONS 64

#define for_each_memblock_region(mrg) \
	for (mrg = memblock.head.next; mrg; mrg = mrg->next)

int memblock_add_region(unsigned long base, unsigned long size);
unsigned long memblock_alloc(unsigned long size);
int memblock_reserve(unsigned long base, unsigned long size);
void memblock_dump_region(void);
