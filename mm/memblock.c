#include <type.h>
#include <printk.h>
#include <page.h>
#include <memblock.h>
#include <bitops.h>

/*
 *  memblock分配器使用memory region的概念来管理内存，它是
 *  在伙伴系统初始化之前使用的这种简单的内存管理。
 *
 *  在初始化伙伴系统时，需要为mem_map[]数组分配内存，但是
 *  由于不知道内存有多少page，所以mem_map[]不能使用全局数
 *  组来分配内存。适合使用memblock来分配内存，这些内存被
 *  设置为RESERVE。
 *
 *  1. 内存状态一共分成两个，一是FREE，另外一个RESERVE。
 *  RESERVE表示已经分配和内核预留的内存，这些内存不会添加
 *  到伙伴系统。
 *  2. 使用一个全局的双向链表来管理所有的memory regions。
 *  所有的regions按照地址从小到大的方向来 串成一个链表。
 *
 *  为了避免嵌套使用memblock分配器，使用memblock_regions[]
 *  来静态初始化n个region描述符，使用的时候用
 *  memblock_get_region_entity()函数来分配。
 *  3. 目前支持的操作：
 *     memblock_add_region() 添加一个memory region到链表里。
 *     memblock_reserve() 把某一个区间设置为RESERVE。
 *     memblock_alloc() 从memblock中分配内存。
 * 4. 由于memblock是临时的内存管理方案，很快就会把空闲内存
 *  交给伙伴系统，因此不支持释放内存操作。
 */

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

		/* 有重叠的地方，说明有bug */
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

		/* 新region在两个region中间 */
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

static void *memblock_alloc(unsigned long size)
{
	unsigned long alloc_start;
	struct memblock_region *mrg, *prev;
	unsigned long mbase, mend;
	struct memblock_region *new;
	unsigned long kernel_end = __pa_symbol((unsigned long)_end);

	size = PAGE_ALIGN_UP(size);

	/*
	 * 从内核image结束的地方开始查找空闲内存
	 */
	alloc_start = PAGE_ALIGN(kernel_end);

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
		if (mbase < alloc_start)
			continue;

		/* 当prev的memblock属性是RESERVE
		 * next的属性是FREE，两者相邻，
		 * 那么可以把prev的memblock往上扩展
		 * 从而分配出空闲页面
		 */
		if (((prev->base + prev->size) == mbase) &&
				prev->flags == MEMBLOCK_RESERVE) {
			prev->size += size;
			mrg->base += size;
			mrg->size -= size;

			return (void *)mbase;
		}

		new = memblock_init_entity(mbase,
				size, MEMBLOCK_RESERVE);
		if (!new)
			return NULL;

		mrg->base = mbase + size;
		mrg->size = mrg->size - size;

		/* insert new node between prev and mrg */
		memblock_insert_new(prev, new, mrg);
		memblock.num_regions++;
		return NULL;
	}

	return NULL;
}

/*
 * memblock_phys_alloc - 返回物理地址
 *
 * 用于线性映射还没建立的场景
 */
void *memblock_phys_alloc(unsigned long size)
{
	return memblock_alloc(size);
}

/*
 * memblock_virt_alloc - 返回线性映射的
 * 虚拟地址
 *
 * 用于线性映射已经建立完成的内存分配
 */
void *memblock_virt_alloc(unsigned long size)
{
	void *phys;

	phys = memblock_phys_alloc(size);

	return __va((unsigned long)phys);
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

		/*case 0: 正好完全吻合 */
		if (mend == end && mbase == base) {
			mrg->flags = MEMBLOCK_RESERVE;
			return 0;
		} else if (mbase == base) {
			/* case 1: 低地址正好吻合，
			 * 高地址处会多出一截
			 */
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
		} else if (mend == base + size) {
			/* case 2: 高地址正好吻合，
			 * 低地址处会多出一截
			 */
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
		} else if (mbase < base && mend > end) {
			/*case 3: 正好在中间，两头都多出一截*/
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

	printk("dump all memblock regions:\n");

	for_each_memblock_region(mrg) {
		printk("0x%08lx - 0x%08lx, flags: %s\n",
				mrg->base, mrg->base + mrg->size,
				mrg->flags ? "RESERVE":"FREE");
	}
}

void reserve_mem_region(unsigned long start, unsigned long end)
{
	unsigned long start_pfn = PFN_DOWN(start);
	unsigned long end_pfn = PFN_UP(end);

	for ( ; start_pfn < end_pfn; start_pfn++) {
		struct page *page = pfn_to_page(start_pfn);

		SetPageReserved(page);
	}
}

static unsigned long memblock_free_memory(unsigned long start,
		unsigned long end)
{
	unsigned long start_pfn = PFN_UP(start);
	unsigned long end_pfn = PFN_DOWN(end);
	unsigned long size;
	int order;

	if (start_pfn >= end_pfn)
		return 0;

	size = end - start;

	while (start_pfn < end_pfn) {
		/* __ffs: 查找start_pfn的第一个bit位的位置
		 * 例如 start_pfn=1, 那么返回0，
		 * start_pfn=2,返回1
		 * start_pfn =0, 返回0xffffffff
		 *
		 * 根据start_pfn与pageblock的对齐和边界关系，
		 * 尽可能确保order取最大的oder值来提交伙伴系统
		 */
		order = min(MAX_ORDER - 1,  __ffs(start_pfn));

		while ((start_pfn + (1 << order)) > end_pfn)
			order--;

		memblock_free_pages(pfn_to_page(start_pfn), order);

		start_pfn += (1 << order);
	}

	return size;
}

unsigned long memblock_free_all(void)
{
	struct memblock_region *mrg;
	unsigned long count = 0;
	unsigned long reserve = 0;

	memblock_dump_region();

	/* reserve pages*/
	for_each_memblock_region(mrg) {
		if (mrg->flags != MEMBLOCK_RESERVE)
			continue;
		reserve_mem_region(mrg->base, mrg->base + mrg->size);
		reserve += mrg->size;
	}

	/* 把memblock管理的空闲页面都添加到伙伴系统中 */
	for_each_memblock_region(mrg) {
		if (mrg->flags != MEMBLOCK_FREE)
			continue;
		count += memblock_free_memory(mrg->base, mrg->base + mrg->size);
	}

	printk("Memory: total %uMB, add %uKB to buddy, %uKB reserve\n",
			(count + reserve)/1024/1024,
			count/1024, reserve/1024);

	show_buddyinfo();

	return count;
}
