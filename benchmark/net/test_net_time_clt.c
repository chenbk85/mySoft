#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <assert.h>

#define DMS(t1, t2)	( ((t2)->tv_sec  - (t1)->tv_sec ) * 1000.0 + \
			  ((t2)->tv_usec - (t1)->tv_usec) / 1000.0 )

struct connection_times {
	struct sockaddr_in addr_local;
	struct sockaddr_in addr_peer;
	struct timeval t_dns_start;
	struct timeval t_dns_end;
	struct timeval t_socket;
	struct timeval t_connection_start;
	struct timeval t_connection_end;
	struct timeval t_sent;
	struct timeval t_first_byte;
	struct timeval t_last_byte;
	struct timeval t_close_end;
};

struct connection_stat_raw {
	long count;
	double tot_total;

	double tot_dns;
	double tot_connect;
	double tot_send;
	double tot_first_byte;
	double tot_rest_byte;
	double tot_close;

	long per_dns;
	long per_connect;
	long per_send;
	long per_first_byte;
	long per_rest_byte;
	long per_close;
	long per_other;
};

struct connection_stat_sum {
	long count;
	double avg_total;
	double avg_dns;
	double avg_connect;
	double avg_send;
	double avg_first_byte;
	double avg_rest_byte;
	double avg_close;

	double per_total;
	double per_dns;
	double per_connect;
	double per_send;
	double per_first_byte;
	double per_rest_byte;
	double per_close;
	double per_other;
};

struct connection_stats {
	time_t time;
	int count;
	struct connection_stat_sum *stats;
};

static struct connection_times *conn_times;
static struct connection_stats *conn_stats;
static const char* host;
static short port = 1433;
static char *buffer;
static size_t size = 2048;
static int running = 1;
static int tcnt = 1;
static int xchg_data = 1;
static int N = 10000; 
static double per = 0.80;
static long gap = 50;
static int interval = 300;
static int count = 1;
static int threshold = 1000;
static const char *detailed_file = "./details.log";
static const char *stat_file = "./stat.log";
static FILE *detailed_fp, *stat_fp;

static void sigint_handler(int sig)
{
	running = 0;
}

int XbsWaitPollable(int fd, short events, long msTimedout)
{
	struct pollfd pfd = { fd, events, 0 };
	int nfd = poll(&pfd, 1, msTimedout);

	if (nfd < 0) return -1;
	if (nfd == 0) return errno = ETIMEDOUT, -1;
	return pfd.revents;
}

int XbsWaitReadable(int fd, long msTimedout)
{
	int event = XbsWaitPollable(fd, POLLIN, msTimedout);
	return (event == -1) ? -1 : 0;
}

int XbsWaitWritable(int fd, long msTimedout)
{
	int event = XbsWaitPollable(fd, POLLOUT, msTimedout);
	return (event == -1) ? -1 : 0;
}

static void set_nonblock(int fd)
{
	int retval, flags = fcntl(fd, F_GETFD);
	flags |= O_NONBLOCK;
	retval = fcntl(fd, F_SETFD, flags);
	assert(retval == 0);
}

static int safe_write(int fd, char* buf, size_t size, long ms)
{
	size_t tlen = 0;
	ssize_t wlen;

	while (tlen < size) {
		if (XbsWaitWritable(fd, ms) < 0) return -1;	
		wlen = write(fd, buf + tlen, size - tlen);
		if (wlen < 0) {
			if (errno == EINTR) continue;
			return -1;
		}

		tlen += wlen;
	}

	return 0;
}

static int safe_read(int fd, char* buf, size_t size, long ms)
{
	size_t tlen = 0;
	ssize_t rlen;

	while (tlen < size) {
		if (XbsWaitReadable(fd, ms) < 0) return -1;
		rlen = read(fd, buf + tlen, size - tlen);
		if (rlen == 0) {
			errno = ECONNRESET;
			return -1;
		}
		else if (rlen < 0) {
			if (errno == EINTR) continue;
			return -1;
		}

		tlen += rlen;
	}

	return 0;
}

static int n2in_addr(const char* host, short port, struct sockaddr_in *addr)
{
	struct addrinfo hint;
	struct addrinfo *result = NULL, *next;
	char serv[NI_MAXSERV];
	int retval;

	memset((void *)&hint, 0, sizeof(hint));
	hint.ai_family = PF_INET;
	hint.ai_flags |= AI_NUMERICSERV;
	hint.ai_socktype = SOCK_STREAM;

	snprintf(serv, sizeof serv, "%d", port);
	if ((retval = getaddrinfo(host, serv, &hint, &result)) < 0) {
		// No such case?
		return -1;
	}
	else if (retval != 0) {
		errno = retval;
		return -1;
	}

	next = result;
	// TODO: find the specified one??

	if (next != NULL) {
		memcpy(addr, next->ai_addr, next->ai_addrlen);
		retval = 0;
	}
	else {
		errno = ENOENT;
		retval = -1;
	}

	freeaddrinfo(result);
	return retval;
}

static char* in_addr2n(struct sockaddr_in* addr, char* buf, size_t blen)
{
	char ibuf[INET_ADDRSTRLEN];

	inet_ntop(addr->sin_family, (void *)&addr->sin_addr, ibuf, sizeof(ibuf));
	snprintf(buf, blen, "%s.%d", ibuf, ntohs(addr->sin_port));
	buf[blen - 1] = 0;
	return buf;
}

static void* thread_entry(void *p)
{
	struct sockaddr_in addr;
	int i, n = (int)(long)p, fd;
	int start = n * (N / tcnt);
	int end = (n == tcnt - 1) ? N : (start + (N / tcnt));
	int retval;

	for (i = start; i < end && running == 1; ++i) {
		gettimeofday(&conn_times[i].t_dns_start, NULL);
		if (n2in_addr(host, port, &addr) < 0) {
			perror("can not resolve host");
			break;
		}
		gettimeofday(&conn_times[i].t_dns_end, NULL);

		gettimeofday(&conn_times[i].t_socket, NULL);
		if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			exit(1);
		}

		gettimeofday(&conn_times[i].t_connection_start, NULL);
		if ((retval = connect(fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
			if (errno == EINPROGRESS) {
				if (XbsWaitWritable(fd, 300000) < 0) {
					perror("waiting for writing");
					exit(3);
				}	
			}
			else {
				perror("connect");
				exit(2);
			}
		}
		gettimeofday(&conn_times[i].t_connection_end, NULL);

		set_nonblock(fd);

		socklen_t alen, alen2;
		alen = alen2 = sizeof conn_times[i].addr_local;
		getsockname(fd, (struct sockaddr *)&conn_times[i].addr_local, &alen);
		getpeername(fd, (struct sockaddr *)&conn_times[i].addr_peer, &alen2);
		
		if (xchg_data) {
			if (safe_write(fd, buffer, size, 300000) < 0) {
				perror("safe_write failed");
				exit(3);
			}
			gettimeofday(&conn_times[i].t_sent, NULL);

			if (XbsWaitReadable(fd, 300000) < 0) {
				perror("waiting for reading failed");
				exit(4);
			}
			gettimeofday(&conn_times[i].t_first_byte, NULL);

			if (safe_read(fd, buffer, size, 300000) < 0) {
				perror("safe_read failed");
				exit(5);
			}
			gettimeofday(&conn_times[i].t_last_byte, NULL);
		}

		close(fd);
		gettimeofday(&conn_times[i].t_close_end, NULL);
	}

	return NULL;
}

static void print_detailed_data(int loop, FILE* fp)
{
	char buf[256], buf2[256];
	int i;

	for (i = 0; i < N; ++i) {
		struct tm tbuf, *ptm = localtime_r(&conn_times[i].t_connection_start.tv_sec, &tbuf);
		fprintf(detailed_fp, "%05d-%05d %s -> %s total: %5.2f dns: %5.2f conn: %5.2f start at %02d:%02d:%02d.%06ld"
		       " send: %5.2f first-byte: %5.2f, rest-bytes: %5.2f close: %5.2f\n",
			loop, i,
			in_addr2n(&conn_times[i].addr_local, buf, sizeof buf),
			in_addr2n(&conn_times[i].addr_peer, buf2, sizeof buf2),
			DMS(&conn_times[i].t_dns_start, &conn_times[i].t_close_end),
			DMS(&conn_times[i].t_dns_start, &conn_times[i].t_dns_end),
			DMS(&conn_times[i].t_connection_start, &conn_times[i].t_connection_end),
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (long)conn_times[i].t_connection_start.tv_usec,	
			DMS(&conn_times[i].t_connection_end, &conn_times[i].t_sent),
			DMS(&conn_times[i].t_sent, &conn_times[i].t_first_byte),
			DMS(&conn_times[i].t_first_byte, &conn_times[i].t_last_byte),
			DMS(&conn_times[i].t_last_byte, &conn_times[i].t_close_end));
	}
}


static void summary_and_print_stat(int loop, time_t time, FILE* fp)
{
	int i, n;
	struct connection_stat_raw *raw;

	// get the maximum time
	double max = 0.0;
	for (i = 0; i < N; ++i) {
		double dms = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_close_end);
		if (max < dms) max = dms;
	}

	n = (int)(max + gap) / gap;
	raw = (struct connection_stat_raw *)calloc(n, sizeof(*raw));

	for (i = 0; i < N; ++i) {
		double dms, tot = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_close_end);
		double threshold = tot * per;
		int bn = 0, idx = (int)tot / gap;

		++raw[idx].count;
		raw[idx].tot_total += tot;
		
		dms = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_dns_end);
		raw[idx].tot_dns += dms;
		if (dms >= threshold) { ++raw[idx].per_dns; ++bn; }

		dms = DMS(&conn_times[i].t_connection_start, &conn_times[i].t_connection_end);
		raw[idx].tot_connect += dms;
		if (dms >= threshold) { ++raw[idx].per_connect; ++bn; }

		dms = DMS(&conn_times[i].t_connection_end, &conn_times[i].t_sent);
		raw[idx].tot_send += dms;
		if (dms >= threshold) { ++raw[idx].per_send; ++bn; }

		dms = DMS(&conn_times[i].t_sent, &conn_times[i].t_first_byte);
		raw[idx].tot_first_byte += dms;
		if (dms >= threshold) { ++raw[idx].per_first_byte; ++bn; }

		dms = DMS(&conn_times[i].t_first_byte, &conn_times[i].t_last_byte);
		raw[idx].tot_rest_byte += dms;
		if (dms >= threshold) { ++raw[idx].per_rest_byte; ++bn; }

		dms = DMS(&conn_times[i].t_last_byte, &conn_times[i].t_close_end);
		raw[idx].tot_close += dms;
		if (dms >= threshold) { ++raw[idx].per_close; ++bn; }

		if (!bn) ++raw[idx].per_other;
	}

	conn_stats[loop].time = time;
	conn_stats[loop].count = n;
	conn_stats[loop].stats = (struct connection_stat_sum *)calloc(n, sizeof *conn_stats[loop].stats);
	if (conn_stats[loop].stats == NULL) {
		perror("calloc failed");
		exit(9);
	}

	for (i = 0; i < n; ++i) {
		if (raw[i].count < 1) continue;
		conn_stats[loop].stats[i].count = raw[i].count;
		conn_stats[loop].stats[i].avg_total = raw[i].tot_total / raw[i].count;
		conn_stats[loop].stats[i].avg_dns = raw[i].tot_dns / raw[i].count;
		conn_stats[loop].stats[i].avg_connect = raw[i].tot_connect / raw[i].count;
		conn_stats[loop].stats[i].avg_send = raw[i].tot_send / raw[i].count;
		conn_stats[loop].stats[i].avg_first_byte = raw[i].tot_first_byte / raw[i].count;
		conn_stats[loop].stats[i].avg_rest_byte = raw[i].tot_rest_byte / raw[i].count;
		conn_stats[loop].stats[i].avg_close = raw[i].tot_close / raw[i].count;

		conn_stats[loop].stats[i].per_total = (double)raw[i].count / N;
		conn_stats[loop].stats[i].per_dns = (double)raw[i].per_dns / raw[i].count;
		conn_stats[loop].stats[i].per_connect = (double)raw[i].per_connect / raw[i].count;
		conn_stats[loop].stats[i].per_send = (double)raw[i].per_send / raw[i].count;
		conn_stats[loop].stats[i].per_first_byte = (double)raw[i].per_first_byte / raw[i].count;
		conn_stats[loop].stats[i].per_rest_byte = (double)raw[i].per_rest_byte / raw[i].count;
		conn_stats[loop].stats[i].per_close = (double)raw[i].per_close / raw[i].count;
		conn_stats[loop].stats[i].per_other = (double)raw[i].per_other / raw[i].count;
	}

	struct tm tbuf, *ptm = localtime_r(&conn_stats[loop].time, &tbuf);
	fprintf(fp, "***** loop: %05d %04d-%02d-%02d %02d:%02d:%02d *****\n", loop,
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	fprintf(fp, "range(ms)\tcount\tpercent(%%)\tavg-total\tavg-dns\tavg-connect\tavg-send\tavg-first-byte\t"
		    "avg-transfer\tavg-close\t\tper-dns(%%)\tper-connect(%%)\tper-send(%%)\tper-first-byte(%%)\t"
		    "per-transfer(%%)\tper-close(%%)\tper-other(%%)\n");
	for (i = 0; i < n; ++i) {
		if (conn_stats[loop].stats[i].count < 1) continue;
		fprintf(fp, "%05d[%ld, %ld)\t%ld\t%5.3f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t"
			    "%5.2f\t%5.2f\t%5.2f\t\t%3.1f\t%3.1f%\t"
			    "%3.1f\t%3.1f\t%3.1f\t%3.1f\t%3.1f\n",
			i, gap * i, gap * (i + 1),
			conn_stats[loop].stats[i].count,
			conn_stats[loop].stats[i].per_total * 100,
			conn_stats[loop].stats[i].avg_total,
			conn_stats[loop].stats[i].avg_dns,
			conn_stats[loop].stats[i].avg_connect,
			conn_stats[loop].stats[i].avg_send,
			conn_stats[loop].stats[i].avg_first_byte,
			conn_stats[loop].stats[i].avg_rest_byte,
			conn_stats[loop].stats[i].avg_close,
			conn_stats[loop].stats[i].per_dns * 100,
			conn_stats[loop].stats[i].per_connect * 100,
			conn_stats[loop].stats[i].per_send * 100,
			conn_stats[loop].stats[i].per_first_byte * 100,
			conn_stats[loop].stats[i].per_rest_byte * 100,
			conn_stats[loop].stats[i].per_close * 100,
			conn_stats[loop].stats[i].per_other * 100);
	}

	free(raw);
	return;
}

static void print_threshold_data(int loop, int threshold, FILE* fp)
{
	int i;
	struct tm tbuf, *ptm = localtime_r(&conn_stats[loop].time, &tbuf);
	fprintf(fp, "***** loop: %05d %04d-%02d-%02d %02d:%02d:%02d *****\n", loop,
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	fprintf(fp, "range(ms)\tcount\tpercent(%%)\tavg-total\tavg-dns\tavg-connect\tavg-send\tavg-first-byte\t"
		    "avg-transfer\tavg-close\t\tper-dns(%%)\tper-connect(%%)\tper-send(%%)\tper-first-byte(%%)\t"
		    "per-transfer(%%)\tper-close(%%)\tper-other(%%)\n");
	for (i = 0; i < conn_stats[loop].count; ++i) {
		if (gap * i < threshold) continue;
		if (conn_stats[loop].stats[i].count < 1) continue;
		fprintf(fp, "%05d[%ld, %ld)\t%ld\t%5.3f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t"
			    "%5.2f\t%5.2f\t%5.2f\t\t%3.1f\t%3.1f\t"
			    "%3.1f\t%3.1f\t%3.1f\t%3.1f\t%3.1f\n",
			i, gap * i, gap * (i + 1),
			conn_stats[loop].stats[i].count,
			conn_stats[loop].stats[i].per_total * 100,
			conn_stats[loop].stats[i].avg_total,
			conn_stats[loop].stats[i].avg_dns,
			conn_stats[loop].stats[i].avg_connect,
			conn_stats[loop].stats[i].avg_send,
			conn_stats[loop].stats[i].avg_first_byte,
			conn_stats[loop].stats[i].avg_rest_byte,
			conn_stats[loop].stats[i].avg_close,
			conn_stats[loop].stats[i].per_dns * 100,
			conn_stats[loop].stats[i].per_connect * 100,
			conn_stats[loop].stats[i].per_send * 100,
			conn_stats[loop].stats[i].per_first_byte * 100,
			conn_stats[loop].stats[i].per_rest_byte * 100,
			conn_stats[loop].stats[i].per_close * 100,
			conn_stats[loop].stats[i].per_other * 100);
	}

	return;
}

static void usage(const void *p)
{
	fprintf(stderr, "Usage: %s options host port\n"
			"  Below are the supported options:\n"
			"    -d ------------------ do not exchanging data.\n"
			"    -t thread-count ----- the number of threads for concurrency. default is 1.\n"
			"    -n test-count ------- the total number of each test-loop will do. default is 10,000.\n"
			"    -s pkg-size --------- the package size when exchange data. default is 2048 bytes.\n"
			"    -i interval --------- run test-loop every \"interval\" seconds. default is 300 seconds.\n"
			"    -c count ------------ run \"count\" times of test-loop. default is 1.\n"
			"    -P percent ---------- classify time consumer into DNS/CONN/BYTE-1/TRANS when the phase spends\n"
			"                          more than the specified \"percent\" of the total time. default is 85.\n"
			"    -g gap -------------- classify response by time step of \"gap\". default is 50ms.\n"
			"    -D detailed file ---- append detailed log into this file. default is \"details.log\".\n"
			"    -S statistics file -- append statistics into this file. default is \"stat.log\".\n"
			"    -f threshold -------- print the statistics which takes longer than that time finally. default is 1000ms.\n"
			"\n", p);
	exit(0);
}

int main(int argc, char **argv) 
{
	struct sigaction sact;
	int ch, retval;
	char *endptr;


	while ((ch = getopt(argc, argv, "dht:n:s:i:c:P:g:D:S:f:")) != EOF) {
		switch (ch) {
		case 'd':
			xchg_data = 0;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 't':
			tcnt = strtol(optarg, &endptr, 0);
			break;
		case 'n':
			N = strtol(optarg, &endptr, 0);
			break;
		case 's':
			size = strtoul(optarg, &endptr, 0);
			break;
		case 'i':
			interval = strtol(optarg, &endptr, 0);
			break;
		case 'c':
			count = strtol(optarg, &endptr, 0);
			break;
		case 'P':
			per = strtol(optarg, &endptr, 0) / 100.0;
			break;
		case 'g':
			gap = strtol(optarg, &endptr, 0);
			break;
		case 'D':
			detailed_file = optarg;
			break;
		case 'S':
			stat_file = optarg;
			break;
		case 'f':
			threshold = strtol(optarg, &endptr, 0);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	
	if ((argc - optind) != 2) {
		usage(argv[0]);
	}
	
	sact.sa_handler = sigint_handler;
	sigfillset(&sact.sa_mask);
	sact.sa_flags = 0;
	sigaction(SIGINT, &sact, NULL);

	host = argv[optind];
	port = strtol(argv[optind + 1], &endptr, 0);

	conn_times = (struct connection_times *)calloc(N, sizeof *conn_times);
	conn_stats = (struct connection_stats *)calloc(count, sizeof *conn_stats);
	buffer = (char *)calloc(size + 16, sizeof(char)); 
	if (conn_times == NULL || conn_stats == NULL || buffer == NULL) {
		perror("allocate memory for conn/stat/buffer failed");
		exit(1);
	}

	detailed_fp = fopen(detailed_file, "a");
	stat_fp = fopen(stat_file, "a");
	if (detailed_fp == NULL || stat_fp == NULL) {
		perror("open details or statistics log file failed");
		exit(2);
	}

	struct timeval tv1, tv2;
	pthread_t tids[tcnt];
	int i, j;
	
	for (j = 0; j < count; ++j) {
		gettimeofday(&tv1, NULL);

		for (i = 0; i < tcnt; ++i) {
			if ((retval = pthread_create(&tids[i], NULL, thread_entry, (void *)(long)i)) != 0) {
				errno = retval; perror("pthread_create");
				exit(2);
			}
		}

		for (i = 0; i < tcnt; ++i) {
			pthread_join(tids[i], NULL);
		}

		print_detailed_data(j, detailed_fp);
		summary_and_print_stat(j, tv1.tv_sec, stat_fp);

		if (j == count - 1) break;

		gettimeofday(&tv2, NULL);
		long us = (tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec);
		if (us < interval * 1000000) {
			struct timespec ts;

			us = interval * 1000000 - us;
			ts.tv_sec = us / 1000000;
			ts.tv_nsec = (us % 1000000) * 1000;
			while (nanosleep(&ts, &ts) != 0) { /* nothing */ };
		}
	}

	for (j = 0; j < count; ++j) {
		print_threshold_data(j, threshold, stdout);	
	}

	return 0;
}
