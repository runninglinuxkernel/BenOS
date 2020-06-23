#ifndef __PRINTK__
#define __PRINTK__
#include <stdarg.h>

int myprintf(char *string, unsigned int size,
		const char *fmt, va_list arg);
int printk(char *fmt, ...);
void print_symbol(unsigned long addr);

void panic(const char *fmt, ...);

#endif
