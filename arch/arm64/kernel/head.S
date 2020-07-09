#include <asm/mm.h>
#include <asm/sysregs.h>
#include <asm/pgtable_prot.h>

.section  .rodata
.align 3
.globl string1
string1:
	.string "Booting at EL"

/* 8字节对齐，否则会出错*/
.align 3
.globl sp_addr
sp_addr:
	.quad init_task_union + THREAD_START_SP

.section ".text.boot"
.globl _start
_start:
	mrs	x0, mpidr_el1
	and	x0, x0,#0xFF		// Check processor id
	cbz	x0, master		// Hang for all non-primary CPU
	b	proc_hang

proc_hang:
	b 	proc_hang

master:
#ifdef CONFIG_DEBUG_ON_EARLY_ASM
        /* init uart and print the string*/
	//bl __init_uart
#endif

	mrs x5, CurrentEL
	cmp x5, #CurrentEL_EL3
	b.eq el3_entry
	b el2_entry

el3_entry:
#ifdef CONFIG_DEBUG_ON_EARLY_ASM
	//bl print_el
#endif
	ldr x0, =SCTLR_EL2_VALUE_MMU_DISABLED
	msr sctlr_el2, x0

	ldr x0, =HCR_HOST_NVHE_FLAGS
	msr hcr_el2, x0

	ldr x0, =SCR_VALUE
	msr scr_el3, x0

	ldr x0, =SPSR_EL2
	msr spsr_el3, x0

	adr x0, el2_entry
	msr elr_el3, x0

	eret

el2_entry:
#ifdef CONFIG_DEBUG_ON_EARLY_ASM
	//bl print_el
#endif
	ldr x0, =SCTLR_EL2_VALUE_MMU_DISABLED
	msr sctlr_el2, x0

	/* The Execution state for EL1 is AArch64 */
	ldr x0, =HCR_HOST_NVHE_FLAGS
	msr hcr_el2, x0

	/* Generic timers. */
	mrs     x0, cnthctl_el2
	orr     x0, x0, #3                      // Enable EL1 physical timers
	msr     cnthctl_el2, x0
	msr     cntvoff_el2, xzr                // Clear virtual offset

	ldr x0, =SCTLR_EL1_VALUE_MMU_DISABLED
	msr sctlr_el1, x0

	ldr x0, =SPSR_EL1
	msr spsr_el2, x0

	adr x0, el1_entry
	msr elr_el2, x0

	eret

el1_entry:
#ifdef CONFIG_DEBUG_ON_EARLY_ASM
	//bl print_el
#endif
	/* 设置异常向量表基地址到vbar寄存器 */
	ldr     x5, =vectors
	msr     vbar_el1, x5
	isb

	adr x0, bss_begin
	adr x1, bss_end
	sub x1, x1, x0
	bl memzero

	/* 创建页表 */
	bl create_page_table
	/* 打开MMU*/
	bl enable_mmu

	/* set sp to top of init_task_union */
	/*
	   这里要用adrp来获取init_task_union的物理地址，
	   不能使用ldr来获取init_task_union的链接地址
	   因为这时 CPU还运行在 VA=PA
	 */
	adrp x2, init_task_union
	add  sp, x2, #THREAD_SIZE

	/*
	   ldr加载C语言入口函数kernel_main的链接地址
	   然后使用br指令来跳转
	   从而实现 重定位功能
	 */
	ldr x2, =kernel_main
	br x2
	b proc_hang // should never come here

enable_mmu:
	tlbi	vmalle1	// Invalidate local TLB
	dsb	nsh

	/*
	  设置MAIR寄存器，用来配置内存属性
	  在页表项中用来设置内存属性
	 */
	ldr	x5, =MAIR(0x00, MT_DEVICE_nGnRnE) | \
		     MAIR(0x04, MT_DEVICE_nGnRE) | \
		     MAIR(0x0c, MT_DEVICE_GRE) | \
		     MAIR(0x44, MT_NORMAL_NC) | \
		     MAIR(0xff, MT_NORMAL) | \
		     MAIR(0xbb, MT_NORMAL_WT)
	msr	mair_el1, x5

	/*
	   这种TCR寄存器，主要设置
	   1. T0SZ： TTBR0页表的管辖大小
	   2. T1SZ： TTBR1页表管辖大小
	   3. TCR_TG0_4K： TTBR0的页表粒度为4KB
	   4. TCR_TG1_4K： TTBR1的页表粒度为4KB
	 */
	ldr	x10, =TCR_TxSZ(VA_BITS) | TCR_TG_FLAGS
	msr	tcr_el1, x10

	ldr x3, =SCTLR_ELx_M

	/*
	   加载两个页表的物理地址
	   恒等映射页表： idmap_pg_dir
	   内核空间映射页表： init_pg_dir
	 */
	adrp x0, idmap_pg_dir
	adrp x1, init_pg_dir
	msr ttbr0_el1, x0
	msr ttbr1_el1, x1
	isb

	/*
	   打开MMU
	 */
	msr sctlr_el1, x3
	isb

	/*
	   无效所有的指令高速缓存
	 */
	ic iallu
	dsb nsh
	isb
	ret

create_page_table:
	mov x25, x30

	/*1. create identical mapping

	  创建恒等映射，即虚拟地址等于物理地址
	  这里把内核image都建立恒等映射
	 */
	adrp  x0, idmap_pg_dir
	adrp  x1, idmap_pg_end
	sub x1, x1, x0
	bl memzero

	/* create idmap mapping */
	adrp x1, _text_boot
	and x1, x1, SECTION_MASK
	mov x2, x1
	adrp x3,  _end
	add x3, x3, #SECTION_SIZE
	sub x3, x3, #1
	and x3, x3, SECTION_MASK
	mov x4, SWAPPER_MM_MMUFLAGS
	adrp  x0, idmap_pg_dir

	bl __create_section_mapping

	/*2. map to link addr

	  把内核image映射到链接地址
	  链接地址为0xffff000010080000
	  在跳转到C语言之前需要把重定位到链接地址
	 */
	adrp  x0, init_pg_dir
	adrp  x1, init_pg_end
	sub x1, x1, x0
	bl memzero

	/* adrp得到了_text_boot的物理地址*/
	adrp x1, _text_boot
	and x1, x1, SECTION_MASK
	/* ldr加载了_text_boot的链接地址*/
	ldr x2, =_text_boot
	ldr x3, =_end
	add x3, x3, #SECTION_SIZE
	sub x3, x3, #1
	and x3, x3, SECTION_MASK
	mov x4, SWAPPER_MM_MMUFLAGS
	adrp  x0, init_pg_dir

	bl __create_section_mapping

	mov x30, x25
	ret

/*
   创建段映射的页表（2MB大小的section）

   x0: page table base address
   x1: phys address
   x2: virt start address
   x3: virt end address
   x4: prot
 */
__create_section_mapping:
	mov x12, x30

	/*
	   第一步： 根据vaddr来找pgd索引，vaddr >> 39
	   结果保存到x8寄存器
	 */
	lsr x8, x2, #PGDIR_SHIFT
	and x8, x8, #PTRS_PER_PGD - 1

	/*
	   第二步： 把pud页表基地址来做成一个pgd页表项
	   填充到刚才找到的pgd表项中

	   x0指向： pgd页表的基地址
	   x0+PAGE_SIZE： 指向PUD页表的基地址
	   x0+2*PAGE_SIZE: 指向PMD页表的基地址
	 */
	add x9, x0, #PAGE_SIZE
	orr x9, x9, #PUD_TYPE_TABLE
	str x9, [x0, x8, lsl #3] // fill pgd entry

	add x0, x0, #PAGE_SIZE

	/*
	   第三步： 根据需要vaddr来找pud索引，vaddr >> 30
	 */
	lsr x8, x2, #PUD_SHIFT
	and x8, x8, #PTRS_PER_PUD - 1

	/*
	   第4步： 把pud页表基地址来做成一个pud页表项
	   填充到刚才找到的pud表项中
	 */
	add x9, x0, #PAGE_SIZE
	orr x9, x9, #PUD_TYPE_TABLE
	str x9, [x0, x8, lsl #3] // fill pud entry
	add x0, x0, #PAGE_SIZE

	/*
	   第5步：
	   根据vaddr来找到pmd索引，然后找到pmd表项
	   由物理地址来构建一个pmd页表项
	   然后填充到pmd表项里
	 */
1:
	lsr x8, x2, #PMD_SHIFT
	and x8, x8, #PTRS_PER_PMD - 1
	lsr x10, x1, #SECTION_SHIFT
	mov x9, x4
	orr x10, x9, x10, lsl #SECTION_SHIFT
	str x10, [x0, x8, lsl #3]

	add x2, x2, #PMD_SIZE  //next
	add x1, x1, #PMD_SIZE  //phy

	cmp x3, x2
	b.hi 1b

	mov x30, x12
	ret

.globl idmap_replace_ttbr1
idmap_replace_ttbr1:
	msr     ttbr1_el1, x0
	isb
	ret

print_el:
	mov x10, x30

	/* print EL */
	ldr x0, =string1
	bl put_string_uart

	mrs x5, CurrentEL
	/* get the currentEL value */
	lsr x2, x5, #2
	mov x0, #48
	add x0, x0, x2
	bl put_uart
	/* print the new line tab */
	mov x0, #10
	bl put_uart

	mov x30, x10
	ret
