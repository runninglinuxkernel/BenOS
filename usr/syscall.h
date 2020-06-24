
extern int __clone(int (*fn)(void *arg), void *child_stack,
		int flags, void *arg);
extern unsigned long syscall(int nr, ...);

int clone(int (*fn)(void *arg), void *child_stack,
		int flags, void *arg);
unsigned long malloc(void);
