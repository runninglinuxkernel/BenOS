#include <asm/sysregs.h>
#include <memory.h>
#include <sched.h>
#include <printk.h>
#include <string.h>
#include <memblock.h>
#include <slab.h>
#include "../usr/syscall.h"

static int el;

static int enable_dump_pgtable = 1;
static int test_virt_access;

static void test_lab2(void)
{
	printk("test lab02: get EL\n");

	printk("running on EL:");
	el = read_sysreg(CurrentEL) >> 2;
	printk("%d\n", el);
}

static void test_lab3(void)
{
	printk("test lab03: printk\n");

	/* test printk */
	printk("el=%3d\n", el);
	printk("el=%-3d\n", el);
	printk("el=%03d\n", el);
	printk("el=0x%-3x\n", el);
	printk("el=0x%03x\n", el);

	printk("0x%hx\n", 0x123456789abcdef);
	printk("0x%x\n", 0x123456789abcdef);
	printk("0x%lx\n", 0x123456789abcdef);
	printk("0x%llx\n", 0x123456789abcdef);

	printk("%d data\n", 0xfffffffe);
	printk("%u data\n", 0xfffffffe);

	printk("%u, %d, %x\n", 1024, -2, -2);
}

static void test_lab04(void)
{
	printk("test lab04: timer\n");
}

static void delay(int n)
{
	while (n--)
		;
}

int kernel_thread1(void *arg)
{
	while (1) {
		delay(50000);
		printk("%s: %s\n", __func__, "12345");
	}

	return 0;
}

int kernel_thread2(void *arg)
{
	while (1) {
		delay(50000);
		printk("%s: %s\n", __func__, "abcde");
	}

	return 0;
}

static int test_lab05(void)
{
	int pid;

	printk("test lab05: fork\n");

	pid = kernel_thread(kernel_thread1, 0, 0);
	if (pid < 0) {
		printk("%s: create thread fail\n", __func__);
		return -EINVAL;
	}

	pid = kernel_thread(kernel_thread2, 0, 0);
	if (pid < 0) {
		printk("%s: create thread fail\n", __func__);
		return -EINVAL;

	}

	return 0;
}

int run_new_user_thread(void *arg)
{
	unsigned long sp;

	while (1) {
		delay(50000);
		asm("mov %0, sp" : "=r" (sp) : : "cc");
		printk("%s: 0x%x\n", __func__, sp);
	}

	return 0;

}

int run_user_thread(void)
{
	unsigned long child_stack;
	int ret;
	unsigned long sp;

	child_stack = malloc();
	if (child_stack < 0)
		printk("cannot allocate memory\n");

	memset((void *)child_stack, 0, PAGE_SIZE);

	printk("child_stack 0x%x\n", child_stack);

	ret = clone(&run_new_user_thread,
			(void *)(child_stack + PAGE_SIZE), 0, NULL);
	if (ret < 0) {
		printk("%s: error while clone\n", __func__);
		return ret;
	}

	while (1) {
		asm("mov %0, sp" : "=r" (sp) : : "cc");
		delay(50000);
		printk("%s: 0x%x\n", __func__, sp);
	}

	return 0;
}

int user_thread(void *arg)
{
	printk("%s: running at EL%d\n", __func__, read_sysreg(CurrentEL) >> 2);

	if (move_to_user_space((unsigned long)&run_user_thread)) {
		printk("error move_to_user_space\n");
		return -EINVAL;
	}

	return 0;
}

static int test_lab08(void)
{
	int pid;

	printk("test lab08: move to userspace\n");

	pid = kernel_thread(user_thread, 0, 0);
	if (pid < 0) {
		printk("%s: create thread fail\n", __func__);
		return -EINVAL;
	}

	return 0;
}

int memblock_test(void)
{
	void *addr;

	memblock_add_region(0, 0x10000000);

	printk("reserver at the middle of memblock\n");
	memblock_reserve(0x100000, 0x400000);
	memblock_dump_region();

	printk("alloc buffer size: 0x200000\n");
	addr = memblock_phys_alloc(0x200000);
	printk("allocated addr: 0x%lx\n", (unsigned long)addr);
	memblock_dump_region();


	printk("reserve at the start of memblock, start 0x700000\n");
	memblock_reserve(0x700000, 0x100000);
	memblock_dump_region();

	printk("reserve at the end of memblock, start 0xfe00000\n");
	memblock_reserve(0xfe00000, 0x200000);
	memblock_dump_region();

	printk("insert a new region at the tail: [0x40000000 ~ 0x50000000]\n");
	memblock_add_region(0x40000000, 0x10000000);
	memblock_dump_region();

	printk("insert a new region at a hole: [0x35000000 ~ 0x36000000]\n");
	memblock_add_region(0x35000000, 0x1000000);
	memblock_dump_region();

#if 0
	printk("insert a new region at overlap: [0x35500000 ~ 0x36500000]\n");
	memblock_add_region(0x35500000, 0x1000000);
	memblock_dump_region();
#endif

	return 0;
}

int test_lab11(void)
{
	//memblock_test();
	memblock_dump_region();

	return 0;
}

int test_memset(void)
{
	char buf[201];
	char val = 0xa5;
	int i;
	char *p;

	memset(buf, val, 201);

	/*check the buffer*/
	p = buf;
	for (i = 0; i < 201; i++) {
		if (*p++ != val)
			printk("%s, failed\n", __func__);
	}

	printk("%s done\n", __func__);

	return 0;
}

int test_slob(void)
{
	void *b;
	int i;
	unsigned int *buffer[100];

	/* test alloc and free*/
	for (i = 0; i < 100000; i++) {
		b = kmalloc(i % 8192);
		if (!b) {
			printk("%s fail, %d\n", __func__, i);
			return -EINVAL;
		}

		kfree(b);
	}

	/* test case 2*/
	for (i = 0; i < 100; i++) {
		b = kmalloc(9);
		if (!b) {
			printk("%s fail, %d\n", __func__, i); return -EINVAL;
		}

		buffer[i] = b;
	}

	kfree(buffer[9]);
	//kfree(buffer[9]);
	kfree(buffer[11]);
	kfree(buffer[9]);

	/* test case 3*/
	b = kmalloc(4 * PAGE_SIZE);
	if (!b) {
		printk("%s fail\n", __func__);
		return -EINVAL;
	}

	printk("%s done\n", __func__);

	return 0;
}

/*
 * 访问一个没有建立映射的地址
 * 应该会触发一级页表访问错误。
 *
 * Translation fault, level 1
 *
 * 见armv8.6手册第2995页
 */
static int test_access_unmap_address(void)
{
	unsigned long address = 0x60000000;

	if (test_virt_access)
		*(unsigned long *)address = 0x55;

	return 0;
}

static void test_dump_pgtable(void)
{
	if (enable_dump_pgtable)
		dump_pgtable();
}

int test_benos(void)
{
	test_lab2();
	test_lab3();
	test_lab04();
	test_lab05();
	test_lab08();

	test_lab11();

	test_memset();
	test_slob();
	test_dump_pgtable();
	test_access_unmap_address();

	return 0;
}
