#include <asm/system.h>
#include <mm.h>
#include <asm/processor.h>

#define NR_TASK 128
#define THREAD_SIZE  (2 * PAGE_SIZE)

enum task_state {
	TASK_RUNNING = 0,
	TASK_INTERRUPTIBLE = 1,
	TASK_UNINTERRUPTIBLE = 2,
	TASK_ZOMBIE = 3,
	TASK_STOPPED = 4,
};

enum task_flags {
	PF_KTHREAD = 1 << 0,
};

struct task_struct {
	enum task_state state;
	enum task_flags flags;
	long count;
	int priority;
	int pid;
	struct cpu_context cpu_context;
};

union task_union {
	struct task_struct task;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

#define INIT_TASK(task) \
{                      \
	.state = 0,     \
	.priority = 1,   \
	.flags = PF_KTHREAD,   \
	.pid = 0,     \
}

extern struct task_struct *g_task[];

extern void ret_from_fork(void);
int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg);
void switch_to(struct task_struct *next);
extern struct task_struct *cpu_switch_to(struct task_struct *prev,
					 struct task_struct *next);

