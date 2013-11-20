#include <pthread.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <malloc.h>
#include <algorithm>
#include <vector>

#define MAX_THREAD_NUM	1024

static int thread_nr = 10;
static bool random_mode = true;

static int read_p = 0;
static int bias_xz = 100;

static int flags = 0;
static off_t fsize = 0; //12ULL*1024ULL*1024ULL*1024ULL;
static size_t bsize = 4096;
static off_t bcount;
static unsigned long long total_limit = 1000ULL * 10000ULL;
static char *buf = NULL;

static const char *file = "./test.data";
static long test_io_cnt = -1;
static int binlog_mode = 0;

static volatile int stop_signal = 0;
static volatile unsigned long thread_ops[MAX_THREAD_NUM];

static volatile long active_thr_cnt = 0;
static pthread_mutex_t active_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t active_cond = PTHREAD_COND_INITIALIZER;

static void exit_thread()
{
	pthread_mutex_lock(&active_mutex);
	--active_thr_cnt;
	pthread_cond_broadcast(&active_cond);
	pthread_mutex_unlock(&active_mutex);
}

static int wait_all_threads_exit(long ms)
{
	struct timeval t1;
	struct timespec t2;
        int all_exit = 0;

	gettimeofday(&t1, NULL);
	t2.tv_sec = t1.tv_sec;
	t2.tv_nsec = t1.tv_usec * 1000 + ms * 1000000;
	t2.tv_sec += t2.tv_nsec / 1000000000;
	t2.tv_nsec %= 1000000000;

	pthread_mutex_lock(&active_mutex);
	while (active_thr_cnt > 0) {
		if (pthread_cond_timedwait(&active_cond, &active_mutex, &t2) == ETIMEDOUT)
			break;
	}

	all_exit = (active_thr_cnt == 0);
	pthread_mutex_unlock(&active_mutex);

	return all_exit ? 0 : -1;
}

static long next_random()
{
	return rand() * rand();

	static int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return rand() * rand();
	long ret;
	if (read(fd, &ret, sizeof(ret)) != sizeof(ret))
		return rand() * rand();
	return ret;
}

static pthread_mutex_t offset_mutex = PTHREAD_MUTEX_INITIALIZER;
static std::vector<off_t> offset_v;
static std::vector<off_t>::iterator offset_i;
static unsigned long long no = 0;

static void binlog_init()
{
	offset_v.reserve(total_limit);
	for (long i = 0; i < total_limit; ++i) {
		off_t offset = next_random();
		if (offset < 0) offset = -offset;
		offset = (offset % bcount) * bsize;
		offset_v.push_back(offset);
	}

	std::sort(offset_v.begin(), offset_v.end());
	offset_i = offset_v.begin();
	return;
}

off_t next_offset()
{
	pthread_mutex_lock(&offset_mutex);
	off_t ret = off_t(-1);

	if (binlog_mode) {
		if (offset_i != offset_v.end()) 
			ret = *offset_i++;
	}
	else if (no < total_limit) {
		ret = next_random();
		if (ret < 0) ret = -ret;
		ret = (ret % bcount) * bsize;
		++no;
	}

	pthread_mutex_unlock(&offset_mutex);
	return ret;
}

static void *thread_entry(void *p)
{
	int index = (long)p;

	int fd = open(file, O_CREAT|O_RDWR|flags, 0644);
	if (fd < 0) {
		fprintf(stderr, "open(%s) failed: %m\n", file);
		goto exit_thr;
	}

	while (stop_signal == 0) {
		off_t offset = next_offset();
		if (offset == off_t(-1)) break;
		if (lseek(fd, offset, SEEK_SET) != offset) {
			fprintf(stderr, "lseek(%s) at %llu failed: %m\n", file, (unsigned long long)offset);
			break;
		}

		int do_r_w = rand() % 100;
		ssize_t wlen;
		if (do_r_w < read_p) {
			wlen = read(fd, buf, bsize);	
		}
		else {
			wlen = write(fd, buf, bsize);
		}

		if (wlen != (ssize_t)bsize) {
			fprintf(stderr, "write(%s) at %llu failed: %d, %m\n", file, (unsigned long long)offset, wlen);
			break;
		}

		++thread_ops[index];
	}

	close(fd);
exit_thr:
	exit_thread();
	return NULL;
}

static void usage(const char *p)
{
	fprintf(stderr, "Usage: %s [ options ]\n", p);
	fprintf(stderr, " Possible options is:\n"
			" -r read-percentage ------------------- percentage of reading\n"
			" -t thread-nr ------------------------- number of thread\n"
			" -b block-size(K/M/G) ----------------- block-size(in direct mode, which msut be 4K*N\n"
			" -R ----------------------------------- do random I/O\n"
			" -D ----------------------------------- do direct I/O (skip RAM)\n"
			" -S ----------------------------------- do ASYNC I/O\n"
			" -T total-times(K/M/G) ---------------- exit when done total times I/O\n"
			" -f data-file ------------------------- data file\n"
			" -s file-size(K/M/G/T) ---------------- set file size(default is file's size\n"
			" -B ----------------------------------- run in binlog mode\n");
	exit(0);
}

int main(int argc, char **argv)
{
	char *endptr;
	int retval, ch;

	while ((ch = getopt(argc, argv, "r:t:b:RDST:f:hs:B")) != EOF) {
		switch (ch) {
		case 'r':
			read_p = atoi(optarg);
			break;
		case 't':
			thread_nr = atoi(optarg);
			break;
		case 'b':
			bsize = strtoull(optarg, &endptr, 0);
			if (*endptr == 'k' || *endptr == 'K') bsize *= 1024;
			else if (*endptr == 'm' || *endptr == 'M') bsize *= 1024 * 1024;
			else if (*endptr == 'g' || *endptr == 'G') bsize *= 1024 * 1024 * 1024;
			break;
		case 'R':
			random_mode = true;
			break;
		case 'D':
			flags |= O_DIRECT;
			break;
		case 'S':
			flags |= O_SYNC;
			break;
		case 'T':
			total_limit = strtoull(optarg, &endptr, 0);
			if (*endptr == 'k' || *endptr == 'K') total_limit *= 1000;
			else if (*endptr == 'm' || *endptr == 'M') total_limit *= 1000000;
			else if (*endptr == 'g' || *endptr == 'G') total_limit *= 1000000000ULL;
			break;
		case 'f':
			file = optarg;
			break;
		case 's':
			fsize = strtoull(optarg, &endptr, 0);
			if (*endptr == 'k' || *endptr == 'K') fsize *= 1024;
			else if (*endptr == 'm' || *endptr == 'M') fsize *= 1024 * 1024;
			else if (*endptr == 'g' || *endptr == 'G') fsize *= 1024 * 1024 * 1024;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 'B':
			binlog_mode = 1;
			break;
		default:
			fprintf(stderr, "Invalid option: %c\n",  ch);
			usage(argv[0]);
			break;
		}
	}

	struct stat stbuf;
	if (stat(file, &stbuf) < 0) {
		fprintf(stderr, "stat(%s) failed: %m\n", file);
		exit(1);
	}

	if (fsize == 0) {
		fsize = stbuf.st_size;
	}
	else {
		fprintf(stdout, " real-fsize: %llu\n", (unsigned long long)stbuf.st_size);
	}

	if ((buf = (char *)memalign(8*1024, bsize)) == NULL) {
		fprintf(stderr, "memalign(bsize=%d) failed: %m\n", bsize);
		exit(2);
	}

	fprintf(stdout, "    read(%): %d\n", read_p);
	fprintf(stdout, "  thread-nr: %d\n", thread_nr);
	fprintf(stdout, " block-size: %d\n", (int)bsize);
	fprintf(stdout, "total-limit: %llu\n", total_limit);
	fprintf(stdout, "       file: %s\n", file);
	fprintf(stdout, "  file-size: %llu (%5.3fG)\n", fsize, (double)fsize/1024.0/1024.0/1024.0);
	fprintf(stdout, "     random: %d\n", random_mode);
	fprintf(stdout, "      flags: 0x%x\n", flags);
	fprintf(stdout, " buffer-ptr: 0x%p\n", (void *)buf);
	fprintf(stdout, "binlog-mode: %d\n", binlog_mode);
	fflush(stdout);

	srand(time(NULL));

	pthread_t tids[MAX_THREAD_NUM];
	struct timeval t1, t2, t3, t4;
	unsigned long long last_count = 0;

	if (thread_nr > MAX_THREAD_NUM) {
		fprintf(stderr, "thread-nr is too many, which must be less than %d\n", MAX_THREAD_NUM);
		exit(2);
	}

	active_thr_cnt = thread_nr;
	bcount = fsize / bsize;
	if (binlog_mode) binlog_init();

	gettimeofday(&t1, NULL);
	for (int i = 0; i < thread_nr; ++i) {
		retval = pthread_create(&tids[i], NULL, thread_entry, (void *)i);
		if (retval != 0) {
			fprintf(stderr, "create thread#%d failed: %s\n", i, strerror(retval));
			exit(2);
		}
	}

	gettimeofday(&t3, NULL);
	while (stop_signal == 0) {
		int all_over = wait_all_threads_exit(10*1000);
		unsigned long long this_count = 0;
		for (int i = 0; i < thread_nr; ++i)
			this_count += thread_ops[i];

		gettimeofday(&t4, NULL);
		long ms = (t4.tv_sec - t3.tv_sec) * 1000 + (t4.tv_usec - t3.tv_usec) / 1000;
		struct tm tbuf, *ptm = localtime_r(&t4.tv_sec, &tbuf);
		unsigned long long this_incr = this_count - last_count;

		fprintf(stdout, "%04d-%02d-%02d %02d:%02d:%02d.%03ld "
				"Interval: %6dms, this/total: %10llu %10llu Rate: %5.3fK/s %5.3fMB/s\n",
			ptm->tm_year + 1900,
			ptm->tm_mon + 1,
			ptm->tm_mday,
			ptm->tm_hour,
			ptm->tm_min,
			ptm->tm_sec,
			(long)t4.tv_usec / 1000,
			ms, this_incr, this_count,
			(double)this_incr / ms,
			(double)this_incr * bsize * 1000.0 / 1048576.0 / ms);
		fflush(stdout);
		last_count = this_count;

		if (last_count >= total_limit || all_over == 0) {
			stop_signal = 99;
			break;
		}

		t3 = t4;
	}

	unsigned long long this_count = 0;
	for (int i = 0; i < thread_nr; ++i) {
		pthread_join(tids[i], NULL);
		this_count += thread_ops[i];
	}

	gettimeofday(&t2, NULL);
	long ms = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000;
	struct tm tbuf, *ptm = localtime_r(&t4.tv_sec, &tbuf);

	fprintf(stdout, "%04d-%02d-%02d %02d:%02d:%02d.%03ld "
			"Interval: %6dms, total: %10llu Rate: %5.3fK/s %5.3fMB/s\n",
		ptm->tm_year + 1900,
		ptm->tm_mon + 1,
		ptm->tm_mday,
		ptm->tm_hour,
		ptm->tm_min,
		ptm->tm_sec,
		(long)t2.tv_usec / 1000,
		ms, this_count,
		(double)this_count / ms,
		(double)this_count * bsize * 1000.0 / 1048576.0 / ms);
	fflush(stdout);

	fprintf(stdout, "Name:\tread-p\tthread-nr\tblock-size\tcount\tfile-size(GB)\trandom\tbinlog-mode\trate\t(#k/s)\trate(MB/s)\n");
	fprintf(stdout, "Total:\t%d\t%d\t%d\t%llu\t%5.3f\t%d\t%d\t%5.3f\t%5.3f\n",
			read_p, thread_nr, (int)bsize, 
			total_limit, 
			(double)fsize/1024.0/1024.0/1024.0,
			random_mode, binlog_mode,
			(double)this_count / ms,
			(double)this_count * bsize * 1000.0 / 1048576.0 / ms);
	fflush(stdout);

	return 0;
}

