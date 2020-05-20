#include <stdarg.h>
#include <asm/uart.h>
#include <type.h>
#include <string.h>

#define CONSOLE_PRINT_BUFFER_SIZE 1024
static char print_buf[CONSOLE_PRINT_BUFFER_SIZE];

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define do_div(n, base) ({					\
	unsigned int __base = (base);				\
	unsigned int  __rem;						\
	__rem = ((u64)(n)) % __base;			\
	(n) = ((u64)(n)) / __base;				\
	__rem;							\
})

static const char *scan_number(const char *string, int *number)
{
	int tmp = 0;

	while (is_digit(*string)) {
		tmp *= 10;
		tmp += *(string++) - '0';
	}

	*number = tmp;
	return string;
}

static char *number(char *str, u64 num, int base, int size, int precision
	, int type)
{
	char c, sign, tmp[36];
	s64 snum;

	if (type & SIGN)
		snum = (s64)num;

	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type & SMALL)
		digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;

	c = (type & ZEROPAD) ? '0' : ' ';

	if (type & SIGN && snum < 0) {
		sign = '-';
		num = -snum;
	} else
		sign = (type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);

	if (sign)
		size--;

	if (type & SPECIAL)
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;


	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = digits[do_div(num, base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;

	if (type & SPECIAL)
		if (base == 8)
			*str++ = '0';
		else if (base == 16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';

	return str;
}

static u64 conver_number(va_list args, int qualifier, int flags)
{
	u64 number = 0;

	if (qualifier == 'L') {
		number = va_arg(args, u64);
		if (flags & SIGN)
			number = (s64)number;
	} else if (qualifier == 'h') {
		number = (u16)va_arg(args, int);
		if (flags & SIGN)
			number = (s16)number;
	} else {
		number = va_arg(args, u32);
		if (flags & SIGN)
			number = (s32)number;
	}

	return number;
}

static int myprintf(char *string, unsigned int size,
		const char *fmt, va_list args)
{
	char *pos;
	int flags;
	int field_width; /* width of output field */
	int precision;
	int qualifier;
	char *ip;
	char *s;
	int i;
	int len;

	pos = string;

	for (; *fmt; fmt++) {
		if (*fmt != '%') {
			*pos++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
repeat:
		++fmt; /* skip first % */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt)) {
			fmt = scan_number(fmt, &field_width);
		} else if (*fmt == '*') {
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				fmt = scan_number(fmt, &precision);
			else if (*fmt == '*')
				precision = va_arg(args, int);
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;

			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				++fmt;
			}
		}

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*pos++ = ' ';
			*pos++ = (unsigned char)va_arg(args, int);
			while (--field_width > 0)
				*pos++ = ' ';
			break;
		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";
			len = strlen(s);
			if (precision < 0)
				precision = len;
			else if (len > precision)
				len = precision;

			if (!(flags & LEFT))
				while (len < field_width--)
					*pos++ = ' ';
			for (i = 0; i < len; i++)
				*pos++ = *s++;
			while (len < field_width--)
				*pos++ = ' ';
			break;

		case 'o':
			pos = number(pos, va_arg(args, unsigned long),
				8, field_width, precision,
				flags);
			break;
		case 'p':
			if (field_width == -1) {
				field_width = 8;
				flags |= ZEROPAD;
			}
			pos = number(pos,
				(unsigned long)va_arg(args, void *),
				16, field_width, precision, flags);
			break;
		case 'x':
			flags |= SMALL;
		case 'X':
			pos = number(pos,
				conver_number(args, qualifier, flags),
				16, field_width, precision, flags);
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			pos = number(pos,
				conver_number(args, qualifier, flags),
				10, field_width, precision, flags);
			break;
		case 'n':
			ip = (char *)va_arg(args, int *);
			*ip = (pos - string);
			break;

		default:
			if (*fmt != '%')
				*pos++ = '%';
			if (*fmt)
				*pos++ = *fmt;
			else
				--fmt;
			break;
		}
	}

	*pos = '\0';
	return pos - string;
}

int printk(const char *fmt, ...)
{
	va_list arg;
	int len;
	int i;

	va_start(arg, fmt);
	len = myprintf(print_buf, sizeof(print_buf), fmt, arg);
	for (i = 0; i < len; i++) {
		putchar(print_buf[i]);
		if (i > sizeof(print_buf))
			break;
	}
	va_end(arg);
	return len;
}
