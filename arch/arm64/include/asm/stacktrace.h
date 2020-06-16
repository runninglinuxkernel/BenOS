
struct stackframe {
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

void dump_backtrace(struct pt_regs *regs, struct task_struct *p);
void show_regs(struct pt_regs *regs);
