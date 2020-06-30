#ifndef ASM_TYPES_H
#define ASM_TYPES_H

#ifdef CONFIG_64BIT
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif /* CONFIG_64BIT */

#endif /*ASM_TYPES_H*/
