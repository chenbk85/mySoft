#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <vector>
#include <cassert>
#include <beyondy/slab_cache.h>
#include <beyondy/slab_cache_t.hpp>

static void usage(const char* p)
{
	fprintf(stderr, "Usage: %s -m | -s sizes-file count\n", p);
	fprintf(stderr, "       %s -l count\n", p);
	exit(0);
}

static std::vector<int> sizes;
static void load_sizes(const char* file)
{
	FILE *fp = fopen(file, "r");
	char line[1024];

	while (fgets(line, sizeof(line), fp) != NULL) {
		int size = strtol(line, NULL, 0);
		sizes.push_back(size);
	}
}

static void test_malloc(int count)
{
	std::vector<int>::iterator iter = sizes.begin();
	for (int i = 0; i < count; ++i) {
		void* addr = malloc(*iter);
		memset(addr, 0, *iter);
		assert(addr != NULL);
		if (++iter == sizes.end())
			iter = sizes.begin();
	}
}

static void test_slab(int count)
{
	std::vector<int>::iterator iter = sizes.begin();

	for (int i = 0; i < count; ++i) {
		void* addr = slab_malloc(*iter);
		memset(addr, 0, *iter);
		assert(addr != NULL);
		if (++iter == sizes.end())
			iter = sizes.begin();
	}

	slab_cache_info(stdout);
}

static void load_test_slab(int count)
{
	std::vector<void *> addrs;

	for (int i = 0; i < count; ++i) {
		int action = rand() % 12;
		if (action < 3) {
			int size = 80; //(rand() % 4095) + 1;
			void *addr = slab_malloc(size);
			if (addr != NULL) addrs.push_back(addr);
//fprintf(stderr, "alloc(%d)=%p\n", (int)size, addr);
		}
		else if (action < 10) {
			long *l = slab_object<long>::instance()->allocate();
			if (l != NULL) addrs.push_back(l);
//fprintf(stderr, "alloc-long=%p\n", l);
		}
		else if (!addrs.empty()) {
			int i = rand() % addrs.size();
			std::vector<void *>::iterator iter = addrs.begin();
			std::advance(iter, i);
//fprintf(stderr, "free(%d)=%p\n", i, *iter);
			assert(*iter != NULL);
			slab_free(*iter);
			addrs.erase(iter);
		}
	}

	slab_cache_info(stdout);
	fprintf(stdout, "########################################\n");

	for (std::vector<void *>::iterator iter = addrs.begin(); iter != addrs.end(); ++iter) {
		slab_free(*iter);
	}

	slab_cache_info(stdout);
	return;
}

static void show_minfo()
{
	struct mallinfo mi = mallinfo();
	fprintf(stderr, "mallinfo: arena=%ld ordblks=%ld smblks=%ld hblks=%ld hblkhd=%ld usmblks=%ld fsmblks=%ld uordblks=%ld fordblks=%ld keepcost=%ld\n", (long)mi.arena, (long)mi.ordblks, (long)mi.smblks, (long)mi.hblks, (long)mi.hblkhd, (long)mi.usmblks, (long)mi.fsmblks, (long)mi.uordblks, (long)mi.fordblks, (long)mi.keepcost);
}

int main(int argc, char **argv)
{
	slab_size_t slab_sizes[] = { 4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52,
				     56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104,
				     108, 112, 116, 120, 124, 128,
				     256, 768, 1024, 1500, 2048, 2500,
				     3000, 3500, 4096, 5500, 6000, 7000, 8192 };
	int retval = slab_cache_init(10, slab_sizes, sizeof(slab_sizes)/sizeof(slab_sizes[0]), NULL, NULL);
	int i, count;

	if (argc == 3 && strcmp(argv[1], "-l") == 0) {
		load_test_slab(atoi(argv[2]));
	}
	else {
		if  (argc != 4) usage(argv[0]);
		load_sizes(argv[2]);
		count = strtol(argv[3], NULL, 0);

		if (strcmp(argv[1], "-m") == 0) {
			fprintf(stdout, "by malloc ...\n");
			test_malloc(count);
		}
		else {
			fprintf(stdout, "by slab ...\n");
			test_slab(count);
		}
	}

	show_minfo();
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "top -p %d", getpid());
	//system(cmd);
	fprintf(stderr, "exit in 30 seconds...\n");
//	sleep(30);
	return 0;
}
