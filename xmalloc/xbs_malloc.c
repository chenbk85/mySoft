#include <string.h>
#include <beyondy/slab_cache.h>

void* malloc(size_t size)
{
	return slab_malloc(size);
}

void* calloc(size_t nmemb, size_t size)
{
	void *ptr = slab_malloc(nmemb * size);
	if (ptr) memset(ptr, 0, nmemb * size);
	return ptr;
}


void free(void *ptr)
{
	slab_free(ptr);
}

void *realloc(void *ptr, size_t size)
{
	return slab_realloc(ptr, size);
}

void malloc_stats()
{
	slab_cache_info(stderr);
}
