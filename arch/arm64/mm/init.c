#include <page.h>
#include <asm/mm.h>
#include <type.h>
#include <memblock.h>
#include <printk.h>

struct pg_data contig_page_data;

extern char _text_boot[], _end[];

unsigned long bootmem_get_start_ddr(void)
{
	return 0;
}

unsigned long bootmem_get_end_ddr(void)
{
	return TOTAL_MEMORY;
}

void memblock_init(void)
{
	unsigned long free;
	unsigned long kernel_size;
	unsigned long start_mem, end_mem;

	start_mem = bootmem_get_start_ddr();
	end_mem = bootmem_get_end_ddr();

	start_mem = PAGE_ALIGN(start_mem);
	end_mem &= PAGE_MASK;
	free = end_mem - start_mem;

	memblock_add_region(start_mem, end_mem);

	kernel_size = _end - _text_boot;

	printk("%s: kernel image: 0x%x - 0x%x, %d\n",
			__func__, (unsigned long)_text_boot,
			(unsigned long)_end, kernel_size);

	memblock_reserve((unsigned long)_text_boot, kernel_size);

	free -= kernel_size;

	printk("Memory: %uKB available, %u free pages, kernel_size: %uKB\n",
			free/1024, free/PAGE_SIZE, kernel_size/1024);

	memblock_dump_region();
}

static void zone_sizes_init(unsigned long min, unsigned long max)
{
	unsigned long zone_size[MAX_NR_ZONES];

	/* we only support one zone now */
	zone_size[ZONE_NORMAL] = max - min;

	free_area_init_node(0, min, zone_size);
}

void bootmem_init(void)
{
	unsigned long min, max;

	min = PFN_UP(bootmem_get_start_ddr());
	max = PFN_DOWN(bootmem_get_end_ddr());

	zone_sizes_init(min, max);
}

void mem_init(void)
{
	memblock_free_all();
}
