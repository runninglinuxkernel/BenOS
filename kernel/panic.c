#include <printk.h>
#include <type.h>
#include <sched.h>
#include <asm/stacktrace.h>

void panic(const char *fmt, ...)
{
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	myprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	printk("Kernel panic: %s\n", buf);
	if (current->pid == 0)
		printk("In idle task - not syncing\n");

	dump_backtrace(NULL, current);

	while (1)
		;
}
