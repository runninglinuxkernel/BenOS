#ifndef ASM_CURRENT_H
#define ASM_CURRENT_H

register unsigned long current_stack_pointer asm ("sp");

static inline struct task_struct *get_current(void)
{
	return (struct task_struct *)
		(current_stack_pointer & ~(THREAD_SIZE - 1));
}

#define current get_current()

#endif /*ASM_CURRENT_H*/
