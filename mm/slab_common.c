#include <slab.h>
#include <list.h>

struct kmem_cache *boot_kmem_cache;
LIST_HEAD(g_slab_caches);

struct kmem_cache *
kmem_cache_create(const char *name, size_t size, size_t align,
		unsigned long flags)
{
	struct kmem_cache *s;
	int ret;

	/* create a kmem_cache */
	s = kmem_cache_zalloc(boot_kmem_cache);
	if (!s)
		goto out;

	s->name = name;
	s->object_size = s->size = size;
	s->align = align;

	ret = __kmem_cache_create(s, flags);
	if (ret)
		goto out_free_cache;

	s->refcount = 1;
	list_add(&s->list, &g_slab_caches);

	return s;

out_free_cache:
	kmem_cache_free(boot_kmem_cache, s);
out:
	return NULL;
}
