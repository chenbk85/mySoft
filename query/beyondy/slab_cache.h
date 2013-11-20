/* slab_cache.h
 * Copyright by beyondy, 2008-2010
 * All rights reserved.
 *
**/
#ifndef __SLAB_CACHE__H
#define __SLAB_CACHE__H

#include <beyondy/list.h>
#include <beyondy/slab_cache.h>
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SLAB_NAMLEN		64

typedef unsigned int slab_size_t;	// do not need too large size_t

typedef struct slab_cache {
	char name[SLAB_NAMLEN];
	pthread_mutex_t lock_;

	struct list_head list_partial;
	struct list_head list_full;
	struct list_head list_free;

	slab_size_t object_size;
	slab_size_t object_align;
	slab_size_t object_num;		// objects per slab
	slab_size_t slab_size;		// slab size

	slab_size_t free_limit;
	slab_size_t free_objects;

	slab_size_t color_next;		// in color-align
	slab_size_t color_range;	// in color-align
	slab_size_t color_align;	// 16
	struct list_head sc_list;
#ifdef ENABLE_STATISTICS
#endif /* ENABLE_STATISTICS */
} slab_cache_t;


int slab_cache_init(slab_size_t __max_blocks, slab_size_t *sizes_size, int sizes_count,
		    void* (*__allocate)(size_t), void (*__deallocate)(void *));
void slab_cache_fini();
void slab_cache_info(FILE* fp);

slab_cache_t* slab_cache_create(const char* name, slab_size_t size, slab_size_t align, slab_size_t maxfree);
void slab_cache_destroy(slab_cache_t* sc);

void* slab_cache_allocate(slab_cache_t *sc);
int slab_cache_free(slab_cache_t* sc, void* addr);

void* slab_malloc(slab_size_t size);
void* slab_remalloc(slab_size_t nsize, void* old, slab_size_t osize);
void slab_free(void* addr);
size_t slab_capacity(void *addr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*! __SLAB_CACHE__H */

