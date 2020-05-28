
#define NR_TASK 128

/* 暂时使用1个4KB页面来当作内核栈*/
#define THREAD_SIZE  (1 * PAGE_SIZE)
#define THREAD_START_SP  (THREAD_SIZE - 8)

#ifndef __ASSEMBLY__

#include <asm/system.h>
#include <mm.h>
#include <asm/processor.h>
#include <list.h>
#include <timer.h>

struct task_struct;
struct run_queue;

struct sched_class {
	const struct sched_class *next;

	void (*enqueue_task)(struct run_queue *rq, struct task_struct *p);
	void (*dequeue_task)(struct run_queue *rq, struct task_struct *p);
	void (*task_tick)(struct run_queue *rq, struct task_struct *p);
	struct task_struct * (*pick_next_task)(struct run_queue *rq,
			struct task_struct *prev);
};

struct run_queue {
	struct list_head rq_head;
	unsigned int nr_running;
	u64 nr_switches;
	struct task_struct *curr;
};

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

/* 进程PCB */
struct task_struct {
	enum task_state state;
	enum task_flags flags;
	int pid;
	struct cpu_context cpu_context;
	struct list_head run_list;
	int counter;
	int priority;
	int need_resched;
	int preempt_count;
	struct task_struct *next_task;
	struct task_struct *prev_task;
};

/*
 * task_struct数据结构存储在内核栈的底部
 */
union task_union {
	struct task_struct task;
	unsigned long stack[THREAD_SIZE/sizeof(long)];
};

#define DEF_COUNTER (HZ/10)  /* 100ms time slice */

/* 0号进程即init进程 */
#define INIT_TASK(task) \
{                      \
	.state = 0,     \
	.priority = 20,   \
	.counter = DEF_COUNTER, \
	.flags = PF_KTHREAD,   \
	.pid = 0,     \
	.preempt_count = 0, \
	.next_task = (struct task_struct *)&task, \
	.prev_task = (struct task_struct *)&task, \
}

extern struct task_struct *g_task[];

extern union task_union init_task_union;

#define init_task (init_task_union.task)

#define SET_LINKS(p) do { \
	(p)->next_task = &init_task; \
	(p)->prev_task = init_task.prev_task; \
	init_task.prev_task->next_task = (p); \
	init_task.prev_task = (p); \
	} while (0)


#define for_each_task(p) \
	for (p = &init_task; (p = p->next_task) != &init_task; )

extern const struct sched_class simple_sched_class;

extern void ret_from_fork(void);
int do_fork(unsigned long clone_flags, unsigned long fn, unsigned long arg);
void switch_to(struct task_struct *next);
extern struct task_struct *cpu_switch_to(struct task_struct *prev,
					 struct task_struct *next);

void sched_init(void);
void schedule(void);
void task_tick(struct run_queue *rq, struct task_struct *p);
void enqueue_task(struct run_queue *rq, struct task_struct *p);
struct task_struct *pick_next_task(struct run_queue *rq,
		struct task_struct *prev);
void tick_handle_periodic(void);
void wake_up_process(struct task_struct *p);

#include <asm/current.h>

static inline void preempt_disable(void)
{
	current->preempt_count++;
}

static inline void preempt_enable(void)
{
	current->preempt_count--;
}

static inline void clear_task_resched(struct task_struct *p)
{
	p->need_resched = 0;
}

#endif  /* __ASSEMBLY__ */
