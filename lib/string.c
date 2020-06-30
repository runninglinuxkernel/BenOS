#include <type.h>
#include <asm/types.h>
#include <asm/string.h>

size_t strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		;

	return sc - s;
}

static void *__memset_1bytes(void *s, int c, size_t count)
{
	char *xs = s;

	while (count--)
		*xs++ = c;
	return s;
}

void *__memset(char *s, int c, size_t count)
{
	char *p = s;
	unsigned long align = sizeof(unsigned long);
	size_t size, left = count;
	int n, i;
	unsigned long address = (unsigned long)p;
	unsigned long data = 0ULL;

	/* 这里c必须转换成unsigned long类型
	 * 否则 只能设置4字节，因为c变量是int类型*/
	for (i = 0; i < align; i++)
		data |= (((unsigned long)c) & 0xff) << (i *8);

	/*1. check start address is align with 8 bytes */
	if (address & (align - 1)) {
		size = address & (align - 1);
		__memset_1bytes(p, c, size);
		p = p + size;
		left = count - size;
	}

	/*align 8 bytes*/
	if (left > align) {
		unsigned long *sc = (unsigned long *)p;

		n = left / align;
		left = left % align;

		while (n--) {
			*sc++ = data;
			p += align;
		}

		if (left)
			__memset_1bytes(p + n * align, c, left);
	}

	return s;
}

#ifndef __HAVE_ARCH_MEMSET
void *memset(void *s, int c, size_t count)
{
	return __memset(s, c, count);
}
#endif

void *memcpy(void *dest, const void *src, size_t count)
{
	char *tmp = dest;
	const char *s = src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}
