/* slab_cache.c
 * Copyright by beyondy, 2008-2010
 * All rights reserved.
 *
**/
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <beyondy/slab_cache.h>
#include <beyondy/rbtree.h>

#ifndef SLAB_BLOCK_OFFSET		// !!!re-define before compiling
#define SLAB_BLOCK_OFFSET	16	// 64K?
#endif /*!SLAB_BLOCK_OFFSET */

#define SLAB_BLOCK_SIZE		(1UL << SLAB_BLOCK_OFFSET)
#define SLAB_BLOCK_MASK		(~(SLAB_BLOCK_SIZE - 1))
#define PTR2SLAB_BLOCK_NO(ptr)	(((unsigned long)(ptr) & SLAB_BLOCK_MASK) >> SLAB_BLOCK_OFFSET)

#ifndef SLAB_USAGE_RATIO
#define SLAB_USAGE_RATIO	95	// !!!re-define before compiling
#endif /*!SLAB_USAGE_RATIO */

#define SLAB_HASH_MAP_SIZE	1024	// default (re-initializable)

// !!!define it as short can support object-size=2
typedef unsigned int slab_bufctl_t;

#define BUFCTL_END	(((slab_bufctl_t)(~0UL))-0)
#define	SLAB_LIMIT	(((slab_bufctl_t)(~0UL))-1)

#define MIN_OBJECT_SIZE	(sizeof(slab_bufctl_t))	// can not be less than
#define MAX_OBJECT_SIZE	((slab_size_t)~0UL)	// why?


typedef struct slab_block {
	struct list_head sb_list;
	slab_size_t object_inuse;
	slab_size_t color_offset;
	slab_bufctl_t next_free;

	slab_cache_t *sc;
	struct rb_node rb_list;
} slab_block_t;

struct sized_slab_cache {
	slab_size_t size;
	slab_cache_t* sc;
};

static struct list_head slab_cache_head = LIST_HEAD_INIT(slab_cache_head);
static struct rb_root slabs_root = RB_ROOT; // all slabs
static struct sized_slab_cache* sized_caches = NULL;
static int sized_count = 0;
static int slab_max_blocks = 4; // def, re-initialiable

static void* (*allocate)(size_t) = NULL;
static void (*deallocate)(void *) = NULL;

static slab_size_t def_sizes_size[] = { 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64,
                                        68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
                                        124, 128, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400,
                                        204800, 409600, 819200 };
static int def_sizes_count = sizeof(def_sizes_size) / sizeof(def_sizes_size[0]);

int slab_cache_init(slab_size_t __max_blocks, slab_size_t *sizes_size, int sizes_count,
		    void *(*__allocate)(size_t), void (*__deallocate)(void *))
{
	int i;

	if ((allocate = __allocate) == NULL)
		allocate = malloc;
	if ((deallocate = __deallocate) == NULL)
		deallocate = free;
	if (sizes_size == NULL || sizes_count < 1) {
		sizes_size = def_sizes_size;
		sizes_count = def_sizes_count;
	}

	slab_max_blocks = __max_blocks;
	sized_count = sizes_count;
	if ((sized_caches = (struct sized_slab_cache *)
			(*allocate)(sizes_count * sizeof(*sized_caches))) == NULL) {
		goto fail;
	}
	
	for (i = 0; i < sizes_count; ++i) {
		assert(sizes_size[i] > 0);
		char name[64];
		snprintf(name, sizeof(name), "size-%ld", (long)sizes_size[i]);
		sized_caches[i].size = sizes_size[i];
		sized_caches[i].sc = slab_cache_create(name, sized_caches[i].size, 0, 0);
		if (sized_caches[i].sc == NULL) {
			goto free_sizes;
		}
	}

	return 0; // all are OK

free_sizes:
	for (/* ** */; i >= 0; --i)
		slab_cache_destroy(sized_caches[i].sc);
	(*deallocate)(sized_caches);
fail:
	return -1;
}

void slab_cache_fini()
{
	int i = 0;
	for (i = 0; i < sized_count; ++i)
		slab_cache_destroy(sized_caches[i].sc);
	(*deallocate)(sized_caches);
}

static void __cache_info(FILE* fp, slab_cache_t* sc, int i)
{
	size_t pc = list_count(&sc->list_partial);
	size_t fc = list_count(&sc->list_full);
	size_t ff = list_count(&sc->list_free);

	double total = (double)(pc + fc + ff) * SLAB_BLOCK_SIZE / 1024.0 / 1024.0;
	double used = (double)((pc + fc + ff) * sc->object_num - sc->free_objects) * sc->object_size / 1024.0 / 1024.0;
	double ratio = 100 * used / ((pc + fc + ff) == 0 ? 1 : total);
	
	fprintf(fp, "*cache[%d]: %s, partial=%ld, full=%ld, free=%ld, object(size=%ld, align=%ld),\n"
		    "    slab(size=%ld, num/p=%ld), free(limit=%ld, num=%ld),\n"
		    "    color(next=%ld, rng=%ld, align=%ld)\n"
		    "    total-allocated=%.3fMb, used=%.3fMb(%.3f%%)\n",
		i, sc->name,
		(long)pc, (long)fc, (long)ff,
		(long)sc->object_size, (long)sc->object_align,
		(long)sc->slab_size, (long)sc->object_num,
		(long)sc->free_limit, (long)sc->free_objects,
		(long)sc->color_next, (long)sc->color_range,
		(long)sc->color_align, total, used, ratio); 
}

void slab_cache_info(FILE* fp)
{
	struct list_head* pl;
	int i;
	fprintf(fp, "slab cache infomation\n"
		    "   allocate: %p%s, deallocate: %p%s, block-size=%d, slab-max-blocks=%d\n"
		    "   sized-caches: [%d] (",
	 	    allocate, allocate == malloc ? "(malloc)" : "",
		    deallocate, deallocate == free ? "(free)" : "",
		    (int)SLAB_BLOCK_SIZE, slab_max_blocks, sized_count);
	for (i = 0; i < sized_count; ++i) {
		fprintf(fp, "%s%ld", (i == 0) ? "" : ", ", (long)sized_caches[i].size);
	}

	fprintf(fp, ")\n");

	i = 0;
	list_for_each(pl, &slab_cache_head) {
		slab_cache_t* sc = list_entry(pl, slab_cache_t, sc_list);
		__cache_info(fp, sc, i++);
	}

	fprintf(fp, "slabs information:\n");
	struct rb_node* pn;
	i = 0;
	for (pn = rb_first(&slabs_root); pn != NULL; pn = rb_next(pn)) {
		slab_block_t* sb = rb_entry(pn, slab_block_t, rb_list);
		fprintf(fp, "slab[%d]: inuse=%ld, offset=%ld, next-free=%ld, sc=%s\n", i++,
			(long)sb->object_inuse, (long)sb->color_offset, (long)sb->next_free, sb->sc->name);
	}

	return;
}

static inline void* index_to_obj(slab_cache_t* sc, slab_block_t* sb, slab_size_t idx)
{
	return (void *)((char *)(sb + 1) + idx * sc->object_size);
}

static slab_size_t obj_to_index(slab_cache_t* sc, slab_block_t* sb, void* objp)
{
	unsigned long idx = (unsigned long)((char *)objp - (char *)(sb + 1));
	idx = idx / sc->object_size;
	assert(idx < (slab_size_t)~0UL);
	return (slab_size_t)idx;
}

static inline slab_block_t* __obj_to_slab(void* obj)
{
	struct rb_node* pn = slabs_root.rb_node;
	unsigned long objp = (unsigned long)obj;
	
	while (pn != NULL) {
		slab_block_t* sb = rb_entry(pn, slab_block_t, rb_list);
		unsigned long sbp = (unsigned long)((char *)sb - sb->color_offset);
		if (objp < sbp)
			pn = pn->rb_left;
		else if (objp >= (sbp + sb->sc->slab_size))
			pn = pn->rb_right;
		else
			return sb;
	}

	return NULL;
}

static inline slab_cache_t* obj_to_cache(void* objp)
{
	slab_block_t* sb = __obj_to_slab(objp);
	if (sb != NULL) return sb->sc;
	return NULL;
}

static inline slab_block_t* obj_to_slab(slab_cache_t* sc, void* objp)
{
	slab_block_t* sb = __obj_to_slab(objp);
	return sb;	
}

static inline void* slab_addr(slab_block_t* sb)
{
	return (void *)((char *)sb - sb->color_offset);
}

static inline slab_block_t* __insert_slab(struct rb_root* root, struct rb_node* node, void* obj)
{
	unsigned long objp = (unsigned long)obj;
	struct rb_node** p = &root->rb_node;
	struct rb_node* parent = NULL;
	slab_block_t* sb;

	while (*p) {
		parent = *p;
		sb = rb_entry(parent, slab_block_t, rb_list);
		unsigned long sbp = (unsigned long)((char *)sb - sb->color_offset);

		if (objp < sbp)
			p = &(*p)->rb_left;
		else if (objp >= (sbp + sb->sc->slab_size))
			p = &(*p)->rb_right;
		else
			return sb;
	}

	rb_link_node(node, parent, p);
	return NULL;
}

static inline int link_slab_list(slab_block_t* sb)
{
	void* obj = (void *)((char *)sb - sb->color_offset);
	slab_block_t* old = __insert_slab(&slabs_root, &sb->rb_list, obj);
	if (old != NULL) {
		assert(old == NULL);
		return -1;
	}

	rb_insert_color(&sb->rb_list, &slabs_root);
	return 0;
}

static inline int unlink_slab_list(slab_block_t* sb)
{
	rb_erase(&sb->rb_list, &slabs_root);
	return 0;
}

static inline slab_bufctl_t* slab_bufctl(slab_block_t* sb, slab_cache_t* sc, slab_size_t idx)
{
	return (slab_bufctl_t *)((char *)(sb + 1) + (idx * sc->object_size));
}

static slab_block_t* slab_cache_grow(slab_cache_t* sc)
{
	assert(sc != NULL);
	void* objp = (*allocate)(sc->slab_size);
	unsigned int offset;
	if (++sc->color_next >= sc->color_range) {
		sc->color_next = 0;
	}

	offset = sc->color_next * sc->color_align;
	slab_block_t* sb = (slab_block_t *)((char *)objp + offset);
	sb->object_inuse = 0;
	sb->color_offset = offset;
	sb->next_free = 0;

	assert(((char *)(sb + 1) + sc->object_num * sc->object_size) <= ((char *)objp + sc->slab_size));

	slab_size_t i = 0;
	for (/* ** */; i < sc->object_num - 1; ++i) {
		*slab_bufctl(sb, sc, i) = i + 1;
	}
	
	*slab_bufctl(sb, sc, i) = BUFCTL_END;

	list_add(&sb->sb_list, &sc->list_free);
	sc->free_objects += sc->object_num;

	sb->sc = sc;
	link_slab_list(sb);
	return sb;
}

static void slab_cache_reap(slab_cache_t* sc)
{
	struct list_head* pl, *n;
	slab_block_t* sb;
	void* addr;
	assert(sc != NULL);

	list_for_each_safe(pl, n, &sc->list_free) {
		if (sc->free_objects < sc->free_limit)
			break;
		sb = list_entry(pl, slab_block_t, sb_list);
		list_del_init(&sb->sb_list);

		assert(sc->free_objects >= sc->object_num);
		sc->free_objects -= sc->object_num;

		unlink_slab_list(sb);
		addr = slab_addr(sb);
		(*deallocate)(addr);
	}

	return;
}

static slab_size_t __calc_slab_sizes(slab_size_t slab_size, slab_size_t object_size, slab_size_t* num_p_slab, int* up)
{
	*num_p_slab = (slab_size - sizeof(slab_block_t)) / object_size;
	*up = *num_p_slab * object_size * 100 / slab_size;
	return (slab_size - sizeof(slab_block_t) - *num_p_slab * object_size);
}

static slab_size_t __estimate_slab_size(size_t size, slab_size_t* num_p_slab, slab_size_t* slab_size)
{
	slab_size_t i, left;
	for (i = 0; i < slab_max_blocks; ++i) {
		int up;
		*slab_size = i * SLAB_BLOCK_SIZE;
		if (*slab_size < (size + sizeof(slab_block_t)))
			continue;
		left = __calc_slab_sizes(*slab_size, size, num_p_slab, &up);
		if (*num_p_slab > 1 && up >= SLAB_USAGE_RATIO)
			break;
	}

	return left;
}

slab_cache_t* slab_cache_create(const char* name, slab_size_t size, slab_size_t align, slab_size_t maxfree)
{
	if (align == 0) {
		// auto determine align
		if (size < 2)
			align = 1;
		else if (size < 4)
			align = 2;
		else if (size < 8)
			align = 4;
		else if (size < 16)
			align = 8;
		else
			align = 16;
	}

	assert(align > 0); // in fact, it must be 2^n=1,2,4,..
	size = (size + align -1) & ~(align - 1);

	// check name
	if (name == NULL || strlen(name) > (SLAB_NAMLEN - 1)) {
		errno = EINVAL;
		return NULL;
	}

	// check object size in [min, max]
	if (size < MIN_OBJECT_SIZE || size > MAX_OBJECT_SIZE) {
		errno = EINVAL;
		return NULL;
	}

	slab_size_t num_p_slab = 0, slab_size, color_align = 16;
	slab_size_t left = __estimate_slab_size(size, &num_p_slab, &slab_size);
	if (num_p_slab == 0)
		return NULL;

	slab_cache_t* sc = (slab_cache_t *)(*allocate)(sizeof(*sc));
	if (sc == NULL)
		return NULL;

	memset(sc, 0, sizeof(*sc));
	strncpy(sc->name, name, sizeof(sc->name));

	INIT_LIST_HEAD(&sc->list_partial);
	INIT_LIST_HEAD(&sc->list_full);
	INIT_LIST_HEAD(&sc->list_free);

	sc->object_size = size;
	sc->object_align = align;
	sc->object_num = num_p_slab;
	sc->slab_size = slab_size;

	sc->free_limit = maxfree;
	sc->free_objects = 0;

	sc->color_next = 0;
	sc->color_align = color_align;
	sc->color_range = left / sc->color_align;

	pthread_mutex_init(&sc->lock_, NULL);

	list_add(&sc->sc_list, &slab_cache_head);
	return sc;
}

static void __destroy_blocks(slab_cache_t* sc, struct list_head* head)
{
	struct list_head* pl, *n;
	list_for_each_safe(pl, n, head) {
		slab_block_t* sb = list_entry(pl, slab_block_t, sb_list);
		void* addr;

		list_del_init(&sb->sb_list);
		unlink_slab_list(sb);

		addr = slab_addr(sb);
		(*deallocate)(addr);
	}
}

void slab_cache_destroy(slab_cache_t* sc)
{
	__destroy_blocks(sc, &sc->list_free);
	__destroy_blocks(sc, &sc->list_partial);
	__destroy_blocks(sc, &sc->list_full);

	list_del(&sc->sc_list);
	(*deallocate)(sc);
}

static inline void* __slab_cache_allocate_partial(slab_block_t* sb, slab_cache_t* sc)
{
	void *objp = index_to_obj(sc, sb, sb->next_free);

	++sb->object_inuse;
	assert(sb->object_inuse > 1 && sb->object_inuse <= sc->object_num);

	assert(sc->free_objects > 0);
	--sc->free_objects;

	sb->next_free = *slab_bufctl(sb, sc, sb->next_free);
	if (sb->next_free == BUFCTL_END) {
		list_del(&sb->sb_list);
		list_add(&sb->sb_list, &sc->list_full);
	}
	else {
		assert(sb->next_free < sc->object_num);
	}

	return objp;
}

static inline void* __slab_cache_allocate_free(slab_block_t* sb, slab_cache_t* sc)
{
	void *objp = index_to_obj(sc, sb, sb->next_free);

	++sb->object_inuse;
	assert(sb->object_inuse == 1);

	assert(sc->free_objects > 0);
	--sc->free_objects; 
	sb->next_free = *slab_bufctl(sb, sc, sb->next_free);
	assert(sb->next_free != BUFCTL_END);

	list_del(&sb->sb_list);
	list_add(&sb->sb_list, &sc->list_partial);

	return objp;
}

void* slab_cache_allocate(slab_cache_t *sc)
{
	assert(sc != NULL);
	void* objp;
	pthread_mutex_lock(&sc->lock_);

	if (!list_empty(&sc->list_partial)) {
		slab_block_t* sb = list_entry(sc->list_partial.next, slab_block_t, sb_list);
		objp = __slab_cache_allocate_partial(sb, sc);
	}
	else if (!list_empty(&sc->list_free)) {
		slab_block_t* sb = list_entry(sc->list_free.next, slab_block_t, sb_list);
		objp = __slab_cache_allocate_free(sb, sc);
	}
	else {
		slab_block_t* sb = slab_cache_grow(sc);
		objp = __slab_cache_allocate_free(sb, sc);
	}
	pthread_mutex_unlock(&sc->lock_);

	return objp;
}

int slab_cache_free(slab_cache_t* sc, void* addr)
{
	slab_block_t* sb;
	slab_size_t idx;
	
	if ((sb = obj_to_slab(sc, addr)) == NULL) {
		return -1;
	}

	idx = obj_to_index(sc, sb, addr);
	*slab_bufctl(sb, sc, idx) = sb->next_free;
	sb->next_free = idx;

	assert(sb->object_inuse > 0 && sb->object_inuse <= sc->object_num);
	--sb->object_inuse;

	++sc->free_objects;
	assert(sc->free_objects > 0);

	if (sb->object_inuse == 0) {
		list_del(&sb->sb_list);
		list_add(&sb->sb_list, &sc->list_free);

		// reap if too many free objects
		if (sc->free_objects > sc->free_limit) {
			slab_cache_reap(sc);
		}
	}

	return 0;
}

void* slab_malloc(slab_size_t size)
{
	slab_size_t i;
	for (i = 0; i < sized_count; ++i) {
		if (size <= sized_caches[i].size) {
			void* addr = slab_cache_allocate(sized_caches[i].sc);
			if (addr != NULL) {
				return addr;
			}

			// continue try? release others and try again?
			return NULL;
		}
	}

	// final, try allocate directly?
	// return (*allocate)(size);
	return NULL;
}

void* slab_remalloc(slab_size_t nsize, void* old, slab_size_t osize)
{
	void* addr = slab_malloc(nsize);
	if (addr != NULL) {
		memcpy(addr, old, nsize > osize ? osize : nsize);
		slab_free(old);
		return addr;
	}

	return NULL;
}

void slab_free(void* addr)
{
	if (addr == NULL)
		return;

	slab_cache_t* sc;
	if ((sc = obj_to_cache(addr)) == NULL
			|| slab_cache_free(sc, addr) < 0) {
		// dangeous??
		assert(0);
		// (*deallocate)(addr);
	}
}

size_t slab_capacity(void *addr)
{
	slab_cache_t* sc = obj_to_cache(addr);
	if (sc != NULL) {
		return sc->object_size;
	}
	else {
		assert(0);
	}

	return 0; // unknow
}

