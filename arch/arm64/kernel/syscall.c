#include <asm/syscall.h>
#include <uapi/syscall.h>
#include <page.h>
#include <fs.h>

static long sys_ni_syscall(struct pt_regs *regs)
{
	return -ENOSYS;
}

static void invoke_syscall(struct pt_regs *regs, int syscall_no,
		int syscall_nr, const syscall_fn_t syscall_table[])
{
	long ret;
	syscall_fn_t fn;

	if (syscall_no < syscall_nr) {
		fn = syscall_table[syscall_no];
		ret = fn(regs);
	} else {
		ret = sys_ni_syscall(regs);
	}

	regs->regs[0] = ret;
}

static void el0_syscall_common(struct pt_regs *regs, int syscall_no,
		int syscall_nr, const syscall_fn_t syscall_table[])
{
	regs->orig_x0 = regs->regs[0];
	regs->syscallno = syscall_no;

	invoke_syscall(regs, syscall_no, syscall_nr, syscall_table);
}

/*
 * 处理系统调用
 * 参数： struct pt_regs *
 */
void el0_svc_handler(struct pt_regs *regs)
{
	return el0_syscall_common(regs, regs->regs[8],
			__NR_syscalls, syscall_table);
}

long __arm64_sys_open(struct pt_regs *regs)
{
	return sys_open((const char *)regs->regs[0],
			regs->regs[1], regs->regs[2]);

}

long __arm64_sys_close(struct pt_regs *regs)
{
	return sys_close(regs->regs[0]);

}

long __arm64_sys_read(struct pt_regs *regs)
{
	return sys_read(regs->regs[0], (char *)regs->regs[1],
			regs->regs[2]);

}

long __arm64_sys_write(struct pt_regs *regs)
{
	return sys_write(regs->regs[0], (char *)regs->regs[1],
			regs->regs[2]);

}

long __arm64_sys_clone(struct pt_regs *regs)
{
	return do_fork(regs->regs[0], regs->regs[1],
			regs->regs[2]);
}

long __arm64_sys_malloc(struct pt_regs *regs)
{
	unsigned long addr;

	addr = get_free_page();
	if (!addr)
		return -ENOMEM;

	return addr;
}

#define __SYSCALL(nr, sym) [nr] = (syscall_fn_t)__arm64_##sym,

/*
 * 创建一个系统调用表syscall_table
 * 每个表项是一个函数指针syscall_fn_t
 */
const syscall_fn_t syscall_table[__NR_syscalls] = {
	__SYSCALL(__NR_open, sys_open)
	__SYSCALL(__NR_close, sys_close)
	__SYSCALL(__NR_read, sys_read)
	__SYSCALL(__NR_write, sys_write)
	__SYSCALL(__NR_clone, sys_clone)
	__SYSCALL(__NR_malloc, sys_malloc)
};
