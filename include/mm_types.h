#ifndef MM_TYPES_H
#define MM_TYPES_H

#include <list.h>
#include <atomic.h>

#define MAX_ORDER 11
#define MAX_ORDER_NR_PAGES (1 << (MAX_ORDER - 1))

extern struct pg_data contig_page_data;
#define NODE_DATA(nid) (&contig_page_data)

enum pageflags {
	PG_locked,
	PG_reserved,
	PG_buddy,
	PG_slab,
};

#define PageReserved(page)  test_bit(PG_reserved, &(page)->flags)
#define SetPageReserved(page)	set_bit(PG_reserved, &(page)->flags)
#define ClearPageReserved(page)	clear_bit(PG_reserved, &(page)->flags)

#define PageLocked(page) test_bit(PG_locked, &(page)->flags)
#define LockPage(page)	set_bit(PG_locked, &(page)->flags)

#define PageBuddy(page)  test_bit(PG_buddy, &(page)->flags)
#define SetPageBuddy(page) set_bit(PG_buddy, &(page)->flags)
#define ClearPageBuddy(page) clear_bit(PG_buddy, &(page)->flags)

#define PageSlab(page)  test_bit(PG_slab, &(page)->flags)
#define SetPageSlab(page) set_bit(PG_slab, &(page)->flags)
#define ClearPageSlab(page) clear_bit(PG_slab, &(page)->flags)

enum zone_type {
	ZONE_NORMAL = 0,
	MAX_NR_ZONES,
};

struct free_area {
	struct list_head free_list;
	unsigned long nr_free;
};

struct zone {
	struct pg_data *zone_pgdata;
	unsigned long zone_start_pfn;
	unsigned long zone_end_pfn;
	unsigned long spanned_pages;
	const char *name;
	struct free_area free_area[MAX_ORDER];
};

struct pg_data {
	struct zone node_zones[MAX_NR_ZONES];
	unsigned long node_start_pfn;
	unsigned long node_spanned_pages;
	int node_id;
	struct page *node_mem_map;
};

struct page {
	unsigned long flags;
	struct list_head lru;
	unsigned long private;
	unsigned int zone_id;
	unsigned int node_id;
	int refcount;
	/*slob*/
	void *freelist;
	int slob_left_units;

};
#endif /*MM_TYPES_H*/
