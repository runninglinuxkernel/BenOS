#include <sched.h>

typedef long (*syscall_fn_t)(struct pt_regs *);

extern const syscall_fn_t syscall_table[];
