#include <uart.h>
#include <asm/sysregs.h>
#include <printk.h>
#include <irq.h>
#include <sched.h>
#include <asm/timer.h>
#include <memory.h>
#include <asm/init.h>
#include <slab.h>
#include "../test/test_kernel.h"

extern char _text_boot[], _etext_boot[];
extern char _text[], _etext[];
extern char _rodata[], _erodata[];
extern char _data[], _edata[];
extern char _bss[], _ebss[];

static void print_mem(void)
{
	printk("BenOS image layout:\n");
	printk("  .text.boot: 0x%08lx - 0x%08lx (%6ld B)\n",
			(u64)_text_boot, (u64)_etext_boot,
			(u32)(_etext_boot - _text_boot));
	printk("       .text: 0x%08lx - 0x%08lx (%6ld B)\n",
			(u64)_text, (u64)_etext,
			(u32)(_etext - _text));
	printk("     .rodata: 0x%08lx - 0x%08lx (%6ld B)\n",
			(u64)_rodata, (u64)_erodata,
			(u32)(_erodata - _rodata));
	printk("       .data: 0x%08lx - 0x%08lx (%6ld B)\n",
			(u64)_data, (u64)_edata,
			(u32)(_edata - _data));
	printk("        .bss: 0x%08lx - 0x%08lx (%6ld B)\n",
			(u64)_bss, (u64)_ebss,
			(u32)(_ebss - _bss));
}

void kernel_main(void)
{
	setup_arch();

	sched_init();
	kmem_cache_init();

	/* print mem layout*/
	print_mem();
	printk("\r\n");

	/* check the 0 thread's task_struct address */
	printk("0 thread's task_struct address: 0x%lx\n",
			&init_task_union.task);
	printk("the SP of 0 thread: 0x%lx\n", current_stack_pointer);

	test_benos();

	timer_init();
	raw_local_irq_enable();

	while (1)
		;
}
