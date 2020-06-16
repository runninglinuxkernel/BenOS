#include <printk.h>
#include <asm/esr.h>
#include <sched.h>
#include <asm/stacktrace.h>

static const char * const bad_mode_handler[] = {
	"Sync Abort",
	"IRQ",
	"FIQ",
	"Error"
};

static const char *esr_class_str[] = {
	[0 ... ESR_ELx_EC_MAX]		= "UNRECOGNIZED EC",
	[ESR_ELx_EC_UNKNOWN]		= "Unknown/Uncategorized",
	[ESR_ELx_EC_WFx]		= "WFI/WFE",
	[ESR_ELx_EC_CP15_32]		= "CP15 MCR/MRC",
	[ESR_ELx_EC_CP15_64]		= "CP15 MCRR/MRRC",
	[ESR_ELx_EC_CP14_MR]		= "CP14 MCR/MRC",
	[ESR_ELx_EC_CP14_LS]		= "CP14 LDC/STC",
	[ESR_ELx_EC_FP_ASIMD]		= "ASIMD",
	[ESR_ELx_EC_CP10_ID]		= "CP10 MRC/VMRS",
	[ESR_ELx_EC_CP14_64]		= "CP14 MCRR/MRRC",
	[ESR_ELx_EC_ILL]		= "PSTATE.IL",
	[ESR_ELx_EC_SVC32]		= "SVC (AArch32)",
	[ESR_ELx_EC_HVC32]		= "HVC (AArch32)",
	[ESR_ELx_EC_SMC32]		= "SMC (AArch32)",
	[ESR_ELx_EC_SVC64]		= "SVC (AArch64)",
	[ESR_ELx_EC_HVC64]		= "HVC (AArch64)",
	[ESR_ELx_EC_SMC64]		= "SMC (AArch64)",
	[ESR_ELx_EC_SYS64]		= "MSR/MRS (AArch64)",
	[ESR_ELx_EC_IMP_DEF]		= "EL3 IMP DEF",
	[ESR_ELx_EC_IABT_LOW]		= "IABT (lower EL)",
	[ESR_ELx_EC_IABT_CUR]		= "IABT (current EL)",
	[ESR_ELx_EC_PC_ALIGN]		= "PC Alignment",
	[ESR_ELx_EC_DABT_LOW]		= "DABT (lower EL)",
	[ESR_ELx_EC_DABT_CUR]		= "DABT (current EL)",
	[ESR_ELx_EC_SP_ALIGN]		= "SP Alignment",
	[ESR_ELx_EC_FP_EXC32]		= "FP (AArch32)",
	[ESR_ELx_EC_FP_EXC64]		= "FP (AArch64)",
	[ESR_ELx_EC_SERROR]		= "SError",
	[ESR_ELx_EC_BREAKPT_LOW]	= "Breakpoint (lower EL)",
	[ESR_ELx_EC_BREAKPT_CUR]	= "Breakpoint (current EL)",
	[ESR_ELx_EC_SOFTSTP_LOW]	= "Software Step (lower EL)",
	[ESR_ELx_EC_SOFTSTP_CUR]	= "Software Step (current EL)",
	[ESR_ELx_EC_WATCHPT_LOW]	= "Watchpoint (lower EL)",
	[ESR_ELx_EC_WATCHPT_CUR]	= "Watchpoint (current EL)",
	[ESR_ELx_EC_BKPT32]		= "BKPT (AArch32)",
	[ESR_ELx_EC_VECTOR32]		= "Vector catch (AArch32)",
	[ESR_ELx_EC_BRK64]		= "BRK (AArch64)",
};

static const char *esr_get_class_string(unsigned int esr)
{
	return esr_class_str[esr >> ESR_ELx_EC_SHIFT];
}

void show_regs(struct pt_regs *regs)
{
	int top_reg = 29;
	int i;

	printk("task: %p\n", current);
	printk("PC is at 0x%016llx\n", regs->pc);
	printk("LR is at 0x%016llx\n", regs->regs[30]);
	printk("pc : [<%016llx>] lr : [<%016llx>] pstate : [<%016llx>]\n",
			regs->pc, regs->regs[30], regs->pstate);

	printk("sp : %016llx\n", regs->sp);

	for (i = top_reg; i >= 0; i--) {
		printk("x%-2d: %016llx ", i, regs->regs[i]);
		if (i%2 == 0)
			printk("\n");
	}

	printk("\n");
}

static int unwind_frame(struct stackframe *frame)
{
	unsigned long high, low;
	unsigned long fp = frame->fp;

	low = frame->sp;
	high = ALIGN(low, THREAD_SIZE);

	if (fp < low || fp > high - 0x18 || fp & 0xf)
		return -EINVAL;

	frame->sp = fp;
	frame->fp = *(unsigned long *)(fp);

	frame->pc = *(unsigned long *)(fp + 8) - 4;

	return 0;
}

void dump_backtrace(struct pt_regs *regs, struct task_struct *p)
{
	struct stackframe frame;

	if (regs) {
		show_regs(regs);

		frame.fp = regs->regs[29];
		frame.sp = regs->sp;
		frame.pc = regs->pc;
	} else if (p == current) {
		frame.fp = (unsigned long)__builtin_frame_address(0);
		frame.sp = current_stack_pointer;
		frame.pc = (unsigned long)dump_backtrace;
	}

	printk("Call trace:\n");
	while (1) {
		unsigned long where = frame.pc;
		int ret;

		ret = unwind_frame(&frame);
		if (ret < 0)
			break;

		print_symbol(where);
	}
}

void bad_mode(struct pt_regs *regs, int reason, unsigned int esr)
{
	printk("Bad mode for %s handler detected, esr:0x%x  - %s\n",
			bad_mode_handler[reason], esr,
			esr_get_class_string(esr));

	dump_backtrace(regs, current);

	while (1)
		;
}
