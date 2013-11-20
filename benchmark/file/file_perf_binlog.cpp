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
#include <algorithm>

#define MAX_THREAD_NUM	1024

static int thread_nr = 10;
static bool random_mode = true;

static int read_p = 0;
static int bias_xz = 100;

static int flags = 0;
static off_t fsize = 0; //12ULL*1024ULL*1024ULL*1024ULL;
static size_t bsize = 4096;
static char *buf = NULL;

static const char *file = "./test.data";

static volatile int stop_signal = 0;
static volatile unsigned long thread_ops[MAX_THREAD_NUM];

static int sync_binlog(const char* bfile)
{
}

static int do_write(void* addr, size_t size)
{
}

static void *thread_entry(void *p)
{
	int index = (long)p;
	off_t bcount = fsize / bsize;

	int fd = open(file, O_CREAT|O_RDWR|flags, 0644);
	if (fd < 0) {
		fprintf(stderr, "open(%s) failed: %m\n", file);
		return NULL;
	}

	while (stop_signal == 0) {
		off_t offset = rand() * rand();
		if (offset < 0) offset = -offset;
		offset = (offset % bcount) * bsize;
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
	return NULL;
}

static void usage(const char *p)
{
	fprintf(stderr, "Usage: %s [ options ]\n", p);
	fprintf(stderr, " Possible options is:\n"
			" -r read-percentage ------------------- percentage of reading\n"
			" -t thread-nr ------------------------- number of thread\n"
			" -b block-size ------------------------ block-size(in direct mode, which msut be 4K*N\n"
			" -R ----------------------------------- do random I/O\n"
			" -D ----------------------------------- do direct I/O (skip RAM)\n"
			" -S ----------------------------------- do ASYNC I/O\n"
			" -T total-times ----------------------- exit when done total times I/O\n"
			" -f data-file ------------------------- data file\n"
			" -s file-size ------------------------- set file size(default is file's size\n");
	exit(0);
}

int main(int argc, char **argv)
{
	unsigned long long total_limit = 1000ULL * 10000ULL;
	char *endptr;
	int retval, ch;

	while ((ch = getopt(argc, argv, "r:t:b:RDST:f:hs:")) != EOF) {
		switch (ch) {
		case 'r':
			read_p = atoi(optarg);
			break;
		case 't':
			thread_nr = atoi(optarg);
			break;
		case 'b':
			bsize = atoi(optarg);
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
		fprintf(stdout, " real-fsize: %llu\n", stbuf.st_size);
	}

	if ((buf = (char *)malloc(bsize + 8*1024)) == NULL) {
		fprintf(stderr, "malloc(bsize=%d) failed: %m\n", bsize);
		exit(2);
	}

	buf = (char *)(((unsigned long long)buf + 8191) & ~8191);
	fprintf(stdout, "    read(%): %d\n", read_p);
	fprintf(stdout, "  thread-nr: %d\n", thread_nr);
	fprintf(stdout, " block-size: %d\n", (int)bsize);
	fprintf(stdout, "total-limit: %llu\n", total_limit);
	fprintf(stdout, "       file: %s\n", file);
	fprintf(stdout, "  file-size: %llu (%5.3fG)\n", fsize, (double)fsize/1024.0/1024.0/1024.0);
	fprintf(stdout, "     random: %d\n", random_mode);
	fprintf(stdout, "      flags: 0x%x\n", flags);
	fprintf(stdout, " buffer-ptr: 0x%p\n", (void *)buf);
	fflush(stdout);

	srand(time(NULL));

	pthread_t tids[MAX_THREAD_NUM];
	struct timeval t1, t2, t3, t4;
	unsigned long long last_count = 0;

	if (thread_nr > MAX_THREAD_NUM) {
		fprintf(stderr, "thread-nr is too many, which must be less than %d\n", MAX_THREAD_NUM);
		exit(2);
	}

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
		sleep(10);
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

		if (last_count >= total_limit) {
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
		(double)this_count * bsize * 1000.0 / 1048576.0);
	fflush(stdout);

	return 0;
}

