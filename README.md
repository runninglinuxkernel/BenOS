# 实验13：实现slob机制
## 1．实验目的
1)	了解slob机制的机制和实现原理。
2)	动手实现slob机制。

## 2．实验要求
1)	在BenOS中实现slob机制，并实现如下接口函数。
```
void *kmalloc(size_t size);
void kfree(const void *block);
```
内核其他模块可以使用kmalloc来分配小块内存。

另外还需要实现如下接口来创建特定的slab缓存，但是slob机制不支持创建特定slab缓存，但是后续我们可以实现功能强大的slab机制或者slub机制。
```
struct kmem_cache * kmem_cache_create(const char *name, size_t size, size_t align,
                     unsigned long flags)
void kmem_cache_destroy(struct kmem_cache *s)
```

2) 写一个测试程序，来测试kmalloc()和kfree()函数功能是否正确。
