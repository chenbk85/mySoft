#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <beyondy/slab_cache.h>

#define MSIZE 8


static void* thread_entry(void *p)
{
	long mcnt = (long)p;
	long i;
	struct timeval t1, t2, t3;
	double ms, ms2;

	slab_malloc(10);
	void **addr = malloc(mcnt * sizeof(void *));

	slab_cache_t *sc = slab_cache_create("80-bytes", MSIZE, 8, 20);
	gettimeofday(&t1, NULL);
	for (i = 0; i < mcnt; ++i) {
		addr[i] = slab_cache_allocate(sc);
		*(char *)addr[i] = 'a';
	}

	gettimeofday(&t2, NULL);
	for (i = 0; i < mcnt; ++i) {
		slab_cache_free(sc, addr[i]);
	}	

	gettimeofday(&t3, NULL);
	ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	ms2 = (t3.tv_sec - t2.tv_sec) * 1000.0 + (t3.tv_usec - t2.tv_usec) / 1000.0;
	fprintf(stdout, "cache-mode: malloc/free %ld times, malloc cost %.2fms, free: %.2fms, total: %.2fms\n", 
		mcnt, ms, ms2, ms + ms2);

	gettimeofday(&t1, NULL);
	for (i = 0; i < mcnt; ++i) {
		addr[i] = slab_malloc(MSIZE);
		*(char *)addr[i] = 'a';
	}

	gettimeofday(&t2, NULL);
	for (i = 0; i < mcnt; ++i) {
		slab_free(addr[i]);
	}	

	gettimeofday(&t3, NULL);
	ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	ms2 = (t3.tv_sec - t2.tv_sec) * 1000.0 + (t3.tv_usec - t2.tv_usec) / 1000.0;
	fprintf(stdout, " slab-mode: malloc/free %ld times, malloc cost %.2fms, free: %.2fms, total: %.2fms\n", 
		mcnt, ms, ms2, ms + ms2);

	gettimeofday(&t1, NULL);
	for (i = 0; i < mcnt; ++i) {
		addr[i] = malloc(MSIZE);
		*(char *)addr[i] = 'a';
	}

	gettimeofday(&t2, NULL);
	for (i = 0; i < mcnt; ++i) {
		free(addr[i]);
	}

	gettimeofday(&t3, NULL);
	ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	ms2 = (t3.tv_sec - t2.tv_sec) * 1000.0 + (t3.tv_usec - t2.tv_usec) / 1000.0;
	fprintf(stdout, "glibc-mode: malloc/free %ld times, malloc cost %.2fms, free: %.2fms, total: %.2fms\n", 
		mcnt, ms, ms2, ms + ms2);
}

int main(int argc, char** argv)
{
	if (argc == 2 && strcmp(argv[1], "-h") == 0) {
		fprintf(stderr, "Usage: %s [ malloc-free-count [ thread-count ] ]\n", argv[0]);
		exit(1);
	}

	slab_cache_init(64);

	pthread_t tids[1000];
	long mcnt = argc >= 2 ? strtol(argv[1], NULL, 0) : 1000000;
	int i, tcnt = argc >= 3 ? strtol(argv[2], NULL, 0) : 2;

	fprintf(stdout, "lood count = %ld, thread count = %ld\n", (long)mcnt, (long)tcnt);

	for (i = 0; i < tcnt; ++i) {
		pthread_create(&tids[i], NULL, thread_entry, (void *)mcnt);
	}

	for (i = 0; i < tcnt; ++i) {
		pthread_join(tids[i], NULL);
	}

	return 0;
}

