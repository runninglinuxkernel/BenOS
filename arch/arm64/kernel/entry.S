#include <asm/asm-offsets.h>

#define BAD_SYNC        0
#define BAD_IRQ         1
#define BAD_FIQ         2
#define BAD_ERROR       3

/* vector table entry */
	.macro vtentry label
	.align 7
	b \label
	.endm

/* handle invlid vector*/
	.macro inv_entry el, reason, regsize = 64
	kernel_entry el, regsize
	mov x0, sp
	mov x1, #\reason
	mrs x2, esr_el1
	b bad_mode
	.endm

/* save the exception context */
	.macro kernel_entry el, regsize = 64
	sub sp, sp, #S_FRAME_SIZE
	.if     \regsize == 32
	mov w0, w0
	.endif

	stp x0, x1, [sp, #16 *0]
	stp x2, x3, [sp, #16 *1]
	stp x4, x5, [sp, #16 *2]
	stp x6, x7, [sp, #16 *3]
	stp x8, x9, [sp, #16 *4]
	stp x10, x11, [sp, #16 *5]
	stp x12, x13, [sp, #16 *6]
	stp x14, x15, [sp, #16 *7]
	stp x16, x17, [sp, #16 *8]
	stp x18, x19, [sp, #16 *9]
	stp x20, x21, [sp, #16 *10]
	stp x22, x23, [sp, #16 *11]
	stp x24, x25, [sp, #16 *12]
	stp x26, x27, [sp, #16 *13]
	stp x28, x29, [sp, #16 *14]

	.if \el == 0
	.else
	add     x21, sp, #S_FRAME_SIZE
	.endif

	mrs     x22, elr_el1
	mrs     x23, spsr_el1
	stp     lr, x21, [sp, #S_LR]
	stp     x22, x23, [sp, #S_PC]

	.endm

/* restore the exception context */
	.macro kernel_exit el, ret = 0
	ldp     x21, x22, [sp, #S_PC]           // load ELR, SPSR

	.if \el == 0
	ldr     x23, [sp, #S_SP]                // load return stack pointer
	msr     sp_el0, x23
	.endif

	msr     elr_el1, x21                    // set up the return data
	msr     spsr_el1, x22

	.if \ret
	ldr     x1, [sp, #S_X1]                 // preserve x0 (syscall return)
	.else
	ldp     x0, x1, [sp, #16 * 0]
	.endif

	ldp     x2, x3, [sp, #16 * 1]
	ldp     x4, x5, [sp, #16 * 2]
	ldp     x6, x7, [sp, #16 * 3]
	ldp     x8, x9, [sp, #16 * 4]
	ldp     x10, x11, [sp, #16 * 5]
	ldp     x12, x13, [sp, #16 * 6]
	ldp     x14, x15, [sp, #16 * 7]
	ldp     x16, x17, [sp, #16 * 8]
	ldp     x18, x19, [sp, #16 * 9]
	ldp     x20, x21, [sp, #16 * 10]
	ldp     x22, x23, [sp, #16 * 11]
	ldp     x24, x25, [sp, #16 * 12]
	ldp     x26, x27, [sp, #16 * 13]
	ldp     x28, x29, [sp, #16 * 14]
	ldr     lr, [sp, #S_LR]
	add     sp, sp, #S_FRAME_SIZE           // restore sp
	eret                                    // return to kernel
	.endm


/* Vector Table */
.align 11
.global vectors
vectors:
	vtentry el1_sync_invalid
	vtentry el1_irq_invalid
	vtentry el1_fiq_invalid
	vtentry el1_error_invalid

	vtentry el1_sync_invalid
	vtentry el1_irq
	vtentry el1_fiq_invalid
	vtentry el1_error_invalid

	vtentry el0_sync_invalid
	vtentry el0_irq
	vtentry el0_fiq_invalid
	vtentry el0_error_invalid

	vtentry el0_sync_invalid
	vtentry el0_irq_invalid
	vtentry el0_fiq_invalid
	vtentry el0_error_invalid

el1_sync_invalid:
	inv_entry 1, BAD_SYNC
el1_irq_invalid:
	inv_entry 1, BAD_IRQ
el1_fiq_invalid:
	inv_entry 1, BAD_FIQ
el1_error_invalid:
	inv_entry 1, BAD_ERROR
el0_sync_invalid:
	inv_entry 0, BAD_SYNC
el0_irq_invalid:
	inv_entry 0, BAD_IRQ
el0_fiq_invalid:
	inv_entry 0, BAD_FIQ
el0_error_invalid:
	inv_entry 0, BAD_ERROR

.align 6
el1_irq:
	kernel_entry 1
	bl irq_handle
	kernel_exit 1

.align 6
el0_irq:
	kernel_entry 0
	bl irq_handle
	kernel_exit 0
