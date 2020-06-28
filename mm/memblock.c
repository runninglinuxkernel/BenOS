#include <type.h>
#include <printk.h>
#include <page.h>
#include <memblock.h>

static struct memblock_region memblock_regions[MAX_MEMBLOCK_REGIONS];

struct memblock memblock = {
	.head.next = NULL,
	.num_regions = 0,
	.total_size = 0,
};

extern char _end[];

static struct memblock_region *memblock_get_region_entity()
{
	int i;

	for (i = 0; i < MAX_MEMBLOCK_REGIONS; i++) {
		if (memblock_regions[i].allocated == 0) {
			memblock_regions[i].allocated = 1;
			return &memblock_regions[i];

		}
	}

	return NULL;
}

static void memblock_free_region_entity(struct memblock_region *mrg)
{
	mrg->allocated = 0;
	mrg->next = mrg->prev = 0;
}

static struct memblock_region *memblock_init_entity(unsigned long base,
		unsigned long size, unsigned int flags)
{
	struct memblock_region *mrg;

	mrg = memblock_get_region_entity();
	if (!mrg)
		return NULL;

	mrg->base = base;
	mrg->size = size;
	mrg->flags = flags;
	mrg->prev = mrg->next = NULL;

	return mrg;
}

/*
 * insert a new entity before mrg
 * @last: the new node insert after this node
 * @new : new entity
 * @mrg : the new insert before this node
 */
#define memblock_insert_new(last, new, mrg) \
	do { \
		last->next = new; \
		new->prev = last; \
		new->next = mrg;  \
		mrg->prev = new; \
	} while (0)

int memblock_add_region(unsigned long base, unsigned long size)
{
	struct memblock_region *mrg, *next, *new;
	unsigned long mbase, mend;
	unsigned long end = base + size;

	base = PAGE_ALIGN_DOWN(base);
	size = PAGE_ALIGN_UP(size);

	new = memblock_init_entity(base, size, MEMBLOCK_FREE);
	if (!new)
		return -EINVAL;

	/* empty memblock*/
	if (memblock.head.next == NULL) {
		memblock.head.next = new;
		new->prev = &memblock.head;

		memblock.total_size = size;
		memblock.num_regions++;
		return 0;
	}

	for_each_memblock_region(mrg) {
		mbase = mrg->base;
		mend = mrg->base + mrg->size;
		next = mrg->next;

		if (mend < base) {
			if (mrg->next)
				continue;
			else
				/*this is last one*/
				goto found_tail;
		}

		if (end > mend && mend > base) {
			printk("%s: error memblock overlap with [0x%lx ~ 0x%lx]\n",
					__func__, mbase, mend);
			break;
		}

		if (next && next->base < end) {
			printk("%s: error memblock overlap with [0x%lx ~ 0x%lx]\n",
					__func__, next->base,
					next->base + next->size);
			break;
		}

		goto found;
	}

	memblock_free_region_entity(new);
	return -EINVAL;

found:
	memblock_insert_new(mrg->prev, new, mrg);
	return 0;

found_tail:
	mrg->next = new;
	new->prev = mrg;
	return 0;
}

unsigned long memblock_alloc(unsigned long size)
{
	unsigned long alloc_start;
	struct memblock_region *mrg, *prev;
	unsigned long mbase, mend;
	struct memblock_region *new;

	size = PAGE_ALIGN_UP(size);

	/* free buffer start from the end of kernel image */
	alloc_start = PAGE_ALIGN((unsigned long)_end);

	for_each_memblock_region(mrg) {
		mbase = mrg->base;
		mend = mrg->base + mrg->size;
		prev = mrg->prev;

		/* find a suit mem region*/
		if (mend <= alloc_start)
			continue;
		if (mrg->size < size)
			continue;
		if (mrg->flags != MEMBLOCK_FREE)
			continue;

		if (mbase >= alloc_start) {
			new = memblock_init_entity(mbase,
					size, MEMBLOCK_RESERVE);
			if (!new)
				return -EINVAL;

			mrg->base = mbase + size;
			mrg->size = mrg->size - size;

			/* insert new node between prev and mrg */
			memblock_insert_new(prev, new, mrg);
			memblock.num_regions++;
			return mbase;
		}
	}

	return -EINVAL;
}

int memblock_reserve(unsigned long base, unsigned long size)
{
	struct memblock_region *mrg, *prev;
	unsigned long mbase, mend;
	struct memblock_region *new, *new1;
	unsigned long end = base + size;

	base = PAGE_ALIGN_DOWN(base);
	size = PAGE_ALIGN_UP(size);

	for (mrg = memblock.head.next; mrg; mrg = mrg->next) {
		mbase = mrg->base;
		mend = mrg->base + mrg->size;
		prev = mrg->prev;

		if (mbase > end)
			break;
		if (mend < base)
			continue;
		if (mrg->flags != MEMBLOCK_FREE)
			continue;

		/*case 0: */
		if (mend == end && mbase == base) {
			mrg->flags = MEMBLOCK_RESERVE;
			return 0;
		} else if (mbase == base) { /* case 1*/
			new = memblock_init_entity(base, size,
					MEMBLOCK_RESERVE);
			if (!new)
				return -EINVAL;

			mrg->base += size;
			mrg->size -= size;

			/* insert new node between prev and mrg */
			memblock_insert_new(prev, new, mrg);
			memblock.num_regions++;
			return 0;
		} else if (mend == base + size) { /* case 2*/
			new = memblock_init_entity(mbase, mrg->size - size,
					MEMBLOCK_FREE);
			if (!new)
				return -EINVAL;

			mrg->base = base;
			mrg->size = size;
			mrg->flags = MEMBLOCK_RESERVE;

			/* insert new node between prev and mrg */
			memblock_insert_new(prev, new, mrg);
			memblock.num_regions++;
			return 0;
		} else if (mbase < base && mend > end) { /*case 3*/
			new = memblock_init_entity(mbase, base - mbase,
					MEMBLOCK_FREE);
			if (!new)
				return -EINVAL;

			new1 = memblock_init_entity(base, size,
					MEMBLOCK_RESERVE);
			if (!new1)
				return -EINVAL;

			mrg->base = base + size;
			mrg->size = mend - new->size - new1->size;
			mrg->flags = MEMBLOCK_FREE;

			/* insert new between prev and new1 */
			memblock_insert_new(prev, new, new1);
			memblock.num_regions++;

			/* insert new1 between new and mrg */
			memblock_insert_new(new, new1, mrg);
			memblock.num_regions++;

			return 0;
		}

	}

	return -EINVAL;
}

void memblock_dump_region(void)
{
	struct memblock_region *mrg;

	for_each_memblock_region(mrg) {
		printk("0x%08lx - 0x%08lx, flags: %s\n",
				mrg->base, mrg->base + mrg->size,
				mrg->flags ? "RESERVE":"FREE");
	}
}
