#ifndef ASM_SYSTEM_H
#define ASM_SYSTEM_H

#include <type.h>

/*
 * PSR bits
 */
#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d
#define PSR_MODE_MASK	0x0000000f

/*
 * pt_regs栈框，用来保存中断现场或者异常现场
 *
 * pt_regs栈框通常位于进程的内核栈的顶部。
 * pt_regs栈框通常位于进程的内核栈的顶部。
 * 而sp的栈顶通常 紧挨着 pt_regs栈框，在pt_regs栈框下方。
 * 保存内容：
 *    x0 ~ x30 通用寄存器
 *    sp
 *    pc
 *    pstate
 */
struct pt_regs {
	struct {
		u64 regs[31];
		u64 sp;
		u64 pc;
		u64 pstate;
	};

	u64 orig_x0;
	u32 syscallno;
	u32 unused2;

	u64 orig_addr_limit;
	u64 unused;
	u64 stackframe[2];
};

#endif /*ASM_SYSTEM_H*/
