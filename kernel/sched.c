#include <sched.h>
#include <irq.h>

struct run_queue g_rq;

struct task_struct *pick_next_task(struct run_queue *rq,
		struct task_struct *prev)
{
	const struct sched_class *class = &simple_sched_class;

	return class->pick_next_task(rq, prev);
}

void dequeue_task(struct run_queue *rq, struct task_struct *p)
{
	const struct sched_class *class = &simple_sched_class;

	return class->dequeue_task(rq, p);
}

void enqueue_task(struct run_queue *rq, struct task_struct *p)
{
	const struct sched_class *class = &simple_sched_class;

	return class->enqueue_task(rq, p);
}

void task_tick(struct run_queue *rq, struct task_struct *p)
{
	const struct sched_class *class = &simple_sched_class;

	return class->task_tick(rq, p);
}

void switch_to(struct task_struct *next)
{
	struct task_struct *prev = current;

	if (prev == next)
		return;

	cpu_switch_to(prev, next);
}

/*
 * 处理调度完成后的一些收尾工作，由next进程来收拾
 * prev进程的烂摊子
 */
void schedule_tail(struct task_struct *prev)
{
	/* 打开中断 */
	raw_local_irq_enable();
}

void tick_handle_periodic(void)
{
	struct run_queue *rq = &g_rq;

	task_tick(rq, current);
}

/* 检查是否在中断上下文里发生了调度，这是一个不好
 * 的习惯。因为中断上下文通常是关中断的，若发生
 * 调度了，CPU选择了next进程运行，CPU就运行在
 * next进程中，那么可能长时间处于关中断状态，这样
 * 时钟tick有可能丢失，导致系统卡住了
 */
static void schedule_debug(struct task_struct *p)
{

}

void schedule(void)
{
	struct task_struct *prev, *next;
	struct run_queue *rq = &g_rq;

	prev = current;

	preempt_disable();

	/* 检查是否在中断上下文中发生了调度 */
	schedule_debug(prev);

	/* 关闭中断包含调度器，以免中断发生影响调度器 */
	raw_local_irq_disable();

	if (prev->state)
		dequeue_task(rq, prev);

	next = pick_next_task(rq, prev);
	clear_task_resched(prev);
	if (next != prev) {
		switch_to(next);
		rq->nr_switches++;
		rq->curr = next;
	}

	/* 由next进程来收拾prev进程的现场 */
	schedule_tail(prev);

	preempt_enable();
}

void preempt_schedule_irq(void)
{
	raw_local_irq_enable();
	schedule();
	raw_local_irq_disable();
}

void wake_up_process(struct task_struct *p)
{
	struct run_queue *rq = &g_rq;

	p->state = TASK_RUNNING;

	enqueue_task(rq, p);
}

void sched_init(void)
{
	struct run_queue *rq = &g_rq;

	INIT_LIST_HEAD(&rq->rq_head);
	rq->nr_running = 0;
	rq->nr_switches = 0;
	rq->curr = NULL;
}
