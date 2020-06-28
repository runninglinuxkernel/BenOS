#include <asm/mm.h>
#include <printk.h>
#include <memblock.h>

extern char _text_boot[], _end[];

void mem_init(unsigned long start_mem, unsigned long end_mem)
{
	unsigned long free;
	unsigned long kernel_size;

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
}

unsigned long get_free_page(void)
{
	return memblock_alloc(PAGE_SIZE);
}

void free_page(unsigned long p)
{
}
