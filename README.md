# 实验12：伙伴系统
## 1．实验目的
1)	了解伙伴系统实现原理。
2)	了解伙伴系统如何分配内存和释放内存

## 2．实验要求
1)	在BenOS中实现伙伴系统，实现如下接口函数。
```
struct page *alloc_pages(unsigned int order);
void __free_pages(struct page *page, unsigned int order);

unsigned long get_free_pages(unsigned int order);
unsigned long get_free_page(void);
void free_pages(unsigned long addr, unsigned int order);
void free_page(unsigned long addr);
```

2) 写一个测试用例来测试页面分配和释放。
