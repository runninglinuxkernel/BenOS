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

static void post_schedule(struct task_struct *prev)
{

}

void tick_handle_periodic(void)
{
	struct run_queue *rq = &g_rq;

	task_tick(rq, current);
}

void schedule(void)
{
	struct task_struct *prev, *next;
	struct run_queue *rq = &g_rq;

	prev = current;

	preempt_disable();

	if (prev->state)
		dequeue_task(rq, prev);

	next = pick_next_task(rq, prev);
	clear_task_resched(prev);
	if (next != prev) {
		switch_to(next);
		rq->nr_switches++;
		rq->curr = next;
	}

	preempt_enable();

	post_schedule(prev);
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
