# 实验11：实现memblock内存管理器
## 1．实验目的
1)	了解memblock实现原理和机制

## 2．实验要求
1)	在BenOS中实现memblock分配器，需要实现如下函数。
    * int memblock_add_region(unsigned long base, unsigned long size);
    * int memblock_reserve(unsigned long base, unsigned long size);
    * void memblock_dump_region(void);
    * void *memblock_alloc(unsigned long size);

2） 写测试程序来检验memblock分配器是否正确。
    
    * 首先加入一大段内存到memblock分配器里。
    * 插入另一段新的内存
    * reserver其中一段区间
    * 分配n个页面
    * dump所有memblock信息查看是否正确
