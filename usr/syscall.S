#include <uapi/syscall.h>

.align 3

/*
  syscall (int nr, ...)

  AArch64 system calls take between 0 and 7 arguments. On entry here nr
  is in w0 and any other system call arguments are in register x1..x7.

  For kernel entry we need to move the system call nr to x8 then
  load the remaining arguments to register.
 */
.global syscall
syscall:
	mov     w8, w0
	mov     x0, x1
	mov     x1, x2
	mov     x2, x3
	mov     x3, x4
	mov     x4, x5
	mov     x5, x6
	mov     x6, x7
	svc     0x0
	ret

/*
   int clone(int (*fn)(void *arg),     x0
            void *child_stack,         x1
	    int flags,                 x2
	    void *arg)                 x3
 */
.global __clone
__clone:
	mov     x10, x0
	mov     x11, x2
	mov     x12, x3

	/* call syscall*/
	mov x0, x2
	/* New sp is already in x1.  */
	mov     x8, #__NR_clone
	svc     0x0

	/*
	  1. 父进程返回时，返回值为 子进程（线程）的pid
	  2. 而子进程返回时， 返回值为0.
	  由于子进程在do_fork期间，直接 把 父进程的
	  pt_regs栈框拷贝过来的, 因此把pt_regs->pc和
	  pt_regs->pstate也拷贝了父进程的， 所以当子进程
	  从内核态返回到用户空间时, 它也是返回到 这里。

	  子进程返回路径：
	  调度器选择子进程 ->
	      进程切换到子进程->
		    ret_from_fork->
		       ret_to_usr->
		               返回到此
	 */
	cmp     x0, #0
	beq     thread_start
	ret

.align 4
thread_start:
	/* Pick the function arg and execute.*/
	mov     x0, x12
	blr     x10

	/* exit */
	ret
