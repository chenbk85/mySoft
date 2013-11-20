#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <vector>

typedef struct free_block_s {
	long blkno;
	int bsize;	/* in K */
	int reserved;
	struct rb_node rb_list;
} free_block_t;

typedef struct allocated_block_s {
	long blkno;
	int size;	/* in byte */
	int reserved;
} allocated_block_t;

static struct rb_root free_blks_root = RB_ROOT;
static free_block_t* next_free_rb = NULL;
static std::map<int, allocated_block_t> dpos;

static char *bbuf;
static char bsize;

static int thread_count = 5;
static int read_percent = 0;
static int interval = 10;	// seconds

#define TMAX 128
enum op_state {
	SUCC = 0,
	FAIL,
	OSBUTT
};

static unsigned long stat_r[TMAX][2];
static unsigned long stat_w[TMAX][2];

static unsigned long stat_scan_free = 0;
static unsigned long stat_scan_split = 0;
static unsigned long stat_alloc_succ = 0;
static unsigned long stat_alloc_fail = 0;
static unsigned long stat_free = 0;
static unsigned long stat_free_merge = 0;

int round_1k(int size)
{
	return (size + 1023) & ~1023;
}

int block_allocate(int key, int size, allocated_block_t& bpos)
{
	int bsize = round_1k(size);
	struct rb_node* pl = next_free_rb;
	while (pl != NULL) {
		free_block_t* fb = rb_entry(pl, free_block_t, rb_list);
		if (fb->bsize >= bsize) {
			bpos.blkno = fb->blkno;
			bpos.size = size;
			if (fb->bsize > bsize) {
				// update it in free-list (its order is kept).
				fb->blkno += bsize;
				fb->bsize -= bsize;

				++stat_scan_split;
			}
			else {
				// remove it from free-list
				struct rb_node* pn = rb_next(pl);
				rb_erase(pl, &free_blks_root);
				if ((next_free_rb = pn) == NULL) {
					next_free_rb = rb_first(&free_blks_root);
				}
			}

			dpos[key] = bpos;
			++stat_alloc_succ;
			return 0;
		}

		pl = rb_next(pl);
		++stat_scan_free;
	}

	++stat_alloc_fail;
	return -1;
}

int block_free(int key, long blkno, int size)
{
	++stat_free_call;
	dpos.erase(key);

	
}

static void usage(const void *p)
{
	fprintf(stderr, "Usage: %s [options]\n"
			"  possible options\n"
			"    -r read-percentage. default is 0.\n"
			"    -b min-block-size(in 1K). default is 1.\n"
			"    -B max-block-size(in 1K). default is 1024.\n"
			"    -f file.\n"
			"    -s file-size. get the size of the above file default.\n"
			"    -d enable direct I/O.\n"
			"    -S enable sync I/O.\n"
			"    -v show verbose statistics.\n"
			"    -h show this help message.\n"
			, p);
	exit(0);
}

static int init_free_list(const char* file, long fsize, int direct, int sync)
{
	int fd = open(file, O_RDWR | O_CREAT | (direct ? O_DIRECT : 0) | (sync ? O_SYNC : 0), 0660);
	if (fd < 0) {
		fprintf(stderr, "open(\"%s\") failed: %s\n", file, strerror(errno));
		return -1;
	}

	if (fsize < 1) {
		struct stat tbuf;
		if (fstat(fd, &tbuf) < 0) {
			fprintf(stderr, "fstat(\"%s\") failed: %s\n", file, strerror(errno));
			goto close_file;
		}

		fsize = tbuf.st_size;
	}

close_file:
	close(fd);
	return -1;
}

static int get_next_key()
{
	pthread_mutex_lock(&kmutex);
	
}

static int is_read_case(int rp)
{
	int per = (int) (100.0 * (rand() / (RAND_MAX + 1.0)));
	return per < rp;
}

static void* thread_loop(void *p)
{
	long tid = (long)p;
	while (status == 0) {
		int nkey = get_next_key();
		if (nkey == NID) break;
		int retval;
		if (is_read_case(read_percent)) {
			retval = do_read(nkey);
			retval == 0 ? ++stat_r[tid][SUCC] : ++stat_r[tid][FAIL];
		}
		else {
			retval = do_write(nkey);
			retval == 0 ? ++stat_w[tid][SUCC] : ++stat_w[tid][FAIL];
		}
	}

	return NULL;
}

static void safe_sleep(long sec)
{
	struct timespec ts = { sec, 0 }, rem;
	while (nanosleep(&ts, &rem)) ts = rem;
}

static void* thread_stat(void *p)
{
	struct timeval t1, t2;
	struct tm tbuf;
	unsigned long prev_r[2], prev_w[2], prev_t;
	unsigned long this_r[2], this_w[2], this_t;
	char buf[1024];

	fprintf(stderr, "time total read:succ fail rate(#k/s) write:succ fail rate(#k/s) scan-depth free-merge\n");

	gettimeofday(&t1, NULL);
	while (status == 0) {
		safe_sleep(interval);
		for (int i = 0; i < OSBUTT; ++i) {
			this_r[i] = this_w[i] = this_t = 0;
		}
		for (int i = 0; i < thread_count; ++i) {
			for (int j = 0; j < OSBUTT; ++j) {
				this_r[j] += stat_r[i][j];
				this_w[j] += stat_w[i][j];
			}
		}
		for (int i = 0; i < OSBUTT; ++i) {
			this_t += stat_r[i] + stat_w[i];
		}

		gettimeofday(&t2, NULL);
		localtime_r(&t2.tv_sec, &tbuf);

		long ms = (t2.tv_sec - t1.tv_sec) * 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
		snprintf(buf, sizeof buf,
			 "%04d-%02d-%02d %02d:%02d:%02d,%03d %8lu %6lu %6lu %5.2f %6lu %6lu %5.2f %3.1f %3.1f",
			tbuf.tm_year + 1900, tbuf.tm_mon + 1, tbuf.tm_mday,
			tbuf.tm_hour, tbuf.tm_min, tbuf.tm_sec, t2.tv_usec / 1000,
			this_t,
			this_r[SUCC] - prev_r[SUCC], this_r[FAIL] - prev_r[FAIL], (this_r[SUCC] - prev_r[SUCC]) / ms,
			this_w[SUCC] - prev_w[SUCC], this_w[FAIL] - prev_w[FAIL], (this_w[SUCC] - prev_w[SUCC]) / ms,
			alloc_scan_depth,
			free_merge_depth);
		fprintf(stderr, "%s\n", buf);
		
		for (int i = 0; i < OSBUTT; ++i) {
			prev_r[i] = this_r[i];
			prev_w[i] = prev_w[i];
		}

		prev_t = this_t;
		t1 = t2;
	}

	return NULL;
}

int main(int argc, char **argv)
{

}
