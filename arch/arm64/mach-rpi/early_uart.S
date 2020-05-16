#include <mach/mini_uart.h>
#include <mach/gpio.h>

string1:
	.ascii "boot EL="
len1:
	.int 8

.align 2
.globl __init_uart
__init_uart:
	ldr x1, =GPFSEL1
	ldr w0, [x1]
	and w0, w0, #0xffff8fff /* selector &= ~(7<<12) */
	orr w0, w0, #0x2000 /* selector |= 2<<12; */
	and w0, w0, #0xfffc7fff  /* selector &= ~(7<<15);*/
	orr w0, w0, #0x10000 /* selector |= 2<<15;*/
	str w0, [x1]

	ldr x1, =GPPUD
	str wzr,[x1]

	/* delay */
	mov x0, #150
1:
	subs x0, x0, #1
	bne 1b

	ldr x1, =GPPUDCLK0
	ldr x2, #0xc000
	str x2, [x1]

	/* delay */
	mov x0, #150
2:
	subs x0, x0, #1
	bne 2b

	ldr x1, =GPPUDCLK0
	str wzr, [x1]

	ldr x1, =AUX_ENABLES
	mov w2, #1
	str w2, [x1]

	ldr x1, =AUX_MU_CNTL_REG
	str wzr, [x1]

	ldr x1, =AUX_MU_CNTL_REG
	str wzr, [x1]

	ldr x1, =AUX_MU_LCR_REG
	mov x2, #3
	str x2, [x1]

	ldr x1, =AUX_MU_BAUD_REG
	mov x2, #270
	str x2, [x1]

	ldr x1, =AUX_MU_CNTL_REG
	mov x2, #3
	str x2, [x1]
	ret

.align 2
.globl put_uart
put_uart:
	ldr x1, =AUX_MU_LSR_REG
1:
	ldr w2, [x1]
	and w2, w2, #0x20
	cmp w2, #0x0
	b.eq 1b

	ldr x1, =AUX_MU_IO_REG
	str x0, [x1]
	ret

.align 2
.globl put_string_uart
put_string_uart:
	ldr x4, =string1
	mov x5, #8
	mov x3, xzr
	/* save to lr register */
	mov x6, x30
1:
	ldrb w0, [x4, x3]
	bl put_uart
	add x3, x3, #1
	cmp x5, x3
	bne 1b

	/* restore the lr and return*/
	mov x30, x6
	ret


