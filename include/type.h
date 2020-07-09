#ifndef _BENOS_TYPE_H
#define _BENOS_TYPE_H

#include <errorno.h>

#define SZ_1K	0x00000400
#define SZ_4K	0x00001000
#define SZ_1M	0x00100000
#define SZ_1G	0x40000000

#define SZ_2M	0x00200000
#define SZ_4M	0x00400000
#define SZ_8M	0x00800000
#define SZ_16M	0x01000000
#define SZ_32M	0x02000000
#define SZ_64M	0x04000000
#define SZ_128M	0x08000000
#define SZ_256M	0x10000000
#define SZ_512M	0x20000000

#define NULL ((void *)0)

#ifdef __ASSEMBLY__
#define _AC(X,Y)	X
#define _AT(T,X)	X
#else
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _AT(T,X)	((T)(X))
#endif

#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))

#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))

#define BIT(nr)		(_UL(1) << (nr))
#define _BITUL(x)	(_UL(1) << (x))
#define _BITULL(x)	(_ULL(1) << (x))

#define GENMASK_ULL(h, l) \
	(((~0ULL) - (1ULL << (l)) + 1) & \
	 (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

#ifndef __ASSEMBLY__
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
#define ALIGN(x, a) __ALIGN_MASK(x, (typeof(x))(a) - 1)

typedef char s8;
typedef unsigned char u8;

typedef short s16;
typedef unsigned short u16;

typedef int s32;
typedef unsigned int u32;

typedef long long s64;
typedef unsigned long long u64;

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#define offsetof(TYPE, MEMBER)	((long)&((TYPE *)0)->MEMBER)

#define min(a, b) (((a) < (b))?(a):(b))
#define max(a, b) (((a) > (b))?(a):(b))

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif
#endif /*BENOS_TYPE_H*/
