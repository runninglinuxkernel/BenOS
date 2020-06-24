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
