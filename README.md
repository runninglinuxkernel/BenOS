# 实验15：实现恒等映射（汇编语言实现）
## 1．实验目的
1)	了解ARM64处理器的内存管理。
2)	了解ARM64处理器的页表管理机制
3)	了解为什么需要恒等映射

## 2．实验要求
1)	在BenOS中使用汇编语言来实现创建页表的函数，把全部的物理内存全部映射到虚拟地址空间。我们建立恒等映射，即物理地址数值上等于虚拟地址。
```
static void __create_pgd_mapping(pgd_t *pgdir, unsigned long phys,
		unsigned long virt, unsigned long size,
		unsigned long prot,
		unsigned long (*alloc_pgtable)(void),
		unsigned long flags)
``` 

2) 实现一个dump页表的函数，把映射的页表打印出来，打印类似这样
```
---[ Identical mapping ]---
0x0000000000080000-0x0000000000081000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000081000-0x0000000000082000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000082000-0x0000000000083000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000083000-0x0000000000084000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000084000-0x0000000000085000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000085000-0x0000000000086000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000086000-0x0000000000087000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000087000-0x0000000000088000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000088000-0x0000000000089000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x0000000000089000-0x000000000008a000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x000000000008a000-0x000000000008b000           4K PTE       ro x  SHD AF            UXN MEM/NORMAL
0x000000000008b000-0x000000000008c000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x000000000008c000-0x000000000008d000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x000000000008d000-0x000000000008e000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x000000000008e000-0x000000000008f000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x000000000008f000-0x0000000000090000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x0000000000090000-0x0000000000091000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x0000000000091000-0x0000000000092000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
0x0000000000092000-0x0000000000093000           4K PTE       RW NX SHD AF            UXN MEM/NORMAL
```
