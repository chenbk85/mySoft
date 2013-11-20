#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
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

enum enum_conn_error {
	E_OK = 0,
	E_DNS,
	E_SOCKET,
	E_CONN_TIMEOUT,
	E_CONN_ERROR,
	E_SEND,
	E_WAITING_READ_TIMEOUT,
	E_READ,
	E_SIZE,
	E_DATA,
	E_BUTT
};

static const struct err_str_map {
	int err_code;
	const char *err_msg;
} err_maps[] = {
	{ E_OK, "OK" },
	{ E_DNS, "DNS" },
	{ E_SOCKET, "SOCKET" },
	{ E_CONN_TIMEOUT, "CONN-TIMEOUT" },
	{ E_CONN_ERROR, "CONN-ERROR" },
	{ E_SEND, "SEND" },
	{ E_WAITING_READ_TIMEOUT, "WAIT-READ-TIMEOUT" },
	{ E_READ, "READ" },
	{ E_SIZE, "SIZE" },
	{ E_DATA, "DATA" },
	{ E_BUTT, NULL }
};

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
	long d_send;
	long d_recv;
	int err_code;
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
static int timeout = 60000;
static const char* host;
static short port = 1433;
static int running = 1;
static int tcnt = 1;
static int N = 10000; 
static double per = 0.80;
static long gap = 50;
static int interval = 300;
static int count = 1;
static int threshold = 1000;
static const char *detailed_file;
static const char *stat_file = "./stat.log";
static FILE *detailed_fp, *stat_fp;

static int exfail = 0;
static int xchg_data = 1;
static char *reqbuf;
static size_t rsize = 0;
static char *rspbuf;
static size_t rsp_buf_size = 1100000;
static size_t min_rsp_size = 0;
static size_t max_rsp_size = 1000000;

static const char *input_file;
static const char *output_file;
static int output_fd = -1;

static const char *url;
static const char *uri;
static int method = 'G'; // G: GET, P: POST, H: HEAD

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
			return (tlen > 0) ? tlen : -1;
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


static int log_inout_data(int loop, int i, const char *err, const char *in, size_t isize, const char *out, size_t osize)
{
	if (output_fd < 0)
		return -1;

	struct iovec iov[5];
	size_t tlen = 0, wlen;
	char buf[128], cr[] = "\n\n";
	int fd, icnt = 0;;

	iov[icnt].iov_base = buf;
	tlen += iov[icnt].iov_len = snprintf(buf, sizeof buf, "*** %05ld-%05ld ***\n", (long)loop, (long)i);
	++icnt;

	if (in == NULL) {
		iov[icnt].iov_base = (void *)err;
		tlen += iov[icnt].iov_len = strlen(err);
		++icnt;
	}
	else {
		iov[icnt].iov_base = (void *)in;
		tlen += iov[icnt].iov_len = isize;
		++icnt;

		if (out == NULL) {
			iov[icnt].iov_base = (void *)err;
			tlen += iov[icnt].iov_len = strlen(err);
			++icnt;
		}
		else {
			iov[icnt].iov_base = (void *)out;
			tlen += iov[icnt].iov_len = osize;
			++icnt;

			if (err != NULL) {
				iov[icnt].iov_base = (void *)err;
				tlen += iov[icnt].iov_len = strlen(err);
				++icnt;
			}
		}
	}

	iov[icnt].iov_base = (void *)cr;
	tlen += iov[icnt].iov_len = sizeof cr - 1;
	++icnt;

	if (writev(output_fd, iov, icnt) != tlen) {
		perror("writev failed");
		return -1;
	}
	
	return 0;
}

static void* thread_entry(void *p)
{
	struct sockaddr_in addr;
	long i, loop = ((long *)p)[0], n = ((long *)p)[1];
	long start = n * (N / tcnt);
	long end = (n == tcnt - 1) ? N : (start + (N / tcnt));
	char err[1024];
	int fd, retval;

	for (i = start; i < end && running == 1; ++i) {

		// DNS
		gettimeofday(&conn_times[i].t_dns_start, NULL);
		if (n2in_addr(host, port, &addr) < 0) {
			snprintf(err, sizeof err, "E_DNS: resolve host failed: %s.", strerror(errno));
			conn_times[i].err_code = E_DNS;
			goto err_before_conn;
		}

		gettimeofday(&conn_times[i].t_dns_end, NULL);


		// socket
		gettimeofday(&conn_times[i].t_socket, NULL);
		if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			snprintf(err, sizeof err, "E_SOCKET: socket failed: %s.", strerror(errno));
			conn_times[i].err_code = E_SOCKET;
			goto err_before_conn;
		}
		else {
			set_nonblock(fd);
		}

		// connect
		gettimeofday(&conn_times[i].t_connection_start, NULL);
		if ((retval = connect(fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
			if (errno == EINPROGRESS) {
				if (XbsWaitWritable(fd, timeout) < 0) {
					snprintf(err, sizeof err, "E_CONN_TIMEOUT: connection failed: %m.", strerror(errno));
					conn_times[i].err_code = E_CONN_TIMEOUT;
					goto err_before_send;
				}
			}
			else {
				snprintf(err, sizeof err, "E_CONN_ERROR: connect failed: %.", strerror(errno));
				conn_times[i].err_code = E_CONN_ERROR;
				goto err_before_send;
			}
		}

		gettimeofday(&conn_times[i].t_connection_end, NULL);

		// record its local/peer address
		socklen_t alen, alen2;
		alen = alen2 = sizeof conn_times[i].addr_local;
		getsockname(fd, (struct sockaddr *)&conn_times[i].addr_local, &alen);
		getpeername(fd, (struct sockaddr *)&conn_times[i].addr_peer, &alen2);
		
		if (url != NULL || xchg_data) {
			// send request
			if (safe_write(fd, reqbuf, rsize, timeout) < 0) {
				snprintf(err, sizeof err, "E_SEND: write failed: %s.", strerror(errno));
				conn_times[i].err_code = E_SEND;
				goto err_before_send;
			}

			gettimeofday(&conn_times[i].t_sent, NULL);
			
			// waiting for the first byte
			if (XbsWaitReadable(fd, timeout) < 0) {
				snprintf(err, sizeof err, "E_WAITING_READ_TIMEOUT: %s.", strerror(errno));
				conn_times[i].err_code = E_WAITING_READ_TIMEOUT;
				goto err_before_read;
			}

			gettimeofday(&conn_times[i].t_first_byte, NULL);

			// read data (transfer)
			if ((retval = safe_read(fd, rspbuf, rsp_buf_size, timeout)) < 1) {
				snprintf(err, sizeof err, "E_READ: %s.", strerror(errno));
				conn_times[i].err_code = E_READ;
				goto err_before_read;
			}

			gettimeofday(&conn_times[i].t_last_byte, NULL);

			// check the response by size range
			if (retval < min_rsp_size || retval > max_rsp_size) {
				snprintf(err, sizeof err, "E_SIZE: rsp-size(%ld) does not belong to [%ld, %ld].", 
					(long)retval, (long)min_rsp_size, (long)max_rsp_size);
				conn_times[i].err_code = E_SIZE;
				goto err_result;
			}

			// TODO: check response by regex

			log_inout_data(loop, i, NULL, reqbuf, rsize, rspbuf, retval);
			goto do_close;
		}
#if 0
		else if (xchg_data) {
			if (safe_write(fd, reqbuf, rsize, timeout) < 0) {
				snprintf(err, sizeof err, "E_SEND: %s.", strerror(errno));
				conn_times[i].err_code = E_SEND;
				goto err_before_read;
			}
			gettimeofday(&conn_times[i].t_sent, NULL);

			if (XbsWaitReadable(fd, timeout) < 0) {
				snprintf(err, sizeof err, "E_WAITING_READ_TIMEOUT: %s.", strerror(errno));
				conn_times[i].err_code = E_WAITING_READ_TIMEOUT;
				goto err_before_read;
			}
			gettimeofday(&conn_times[i].t_first_byte, NULL);

			if ((retval = safe_read(fd, rspbuf, rsp_buf_size, timeout)) < 0) {
				snprintf(err, sizeof err, "E_READ: %s.", strerror(errno));
				conn_times[i].err_code = E_READ;
				goto err_before_read;
			}
			gettimeofday(&conn_times[i].t_last_byte, NULL);

			if (retval < min_rsp_size || retval > max_rsp_size) {
				snprintf(err, sizeof err, "E_SIZE: rsp-size(%ld) does not belong to [%ld, %ld].", 
					(long)retval, (long)min_rsp_size, (long)max_rsp_size);
				conn_times[i].err_code = E_SIZE;
				goto err_result;
			}

			log_inout_data(loop, i, NULL, reqbuf, rsize, rspbuf, retval);
			goto do_close;
		}
#endif
	
	err_before_send:	
		err[sizeof err - 1] = 0;
		log_inout_data(loop, i, err, NULL, 0, NULL, 0);
		goto do_close;
	err_before_read:
		err[sizeof err - 1] = 0;
		log_inout_data(loop, i, err, reqbuf, rsize, NULL, 0);
		goto do_close;
	err_result:
		log_inout_data(loop, i, err, reqbuf, rsize, rspbuf, retval);
		// fall through ...
	do_close:
		close(fd);
		gettimeofday(&conn_times[i].t_close_end, NULL);
		continue;
	err_before_conn:
		err[sizeof err - 1] = 0;
		log_inout_data(loop, i, err, NULL, 0, NULL, 0);
	}

	return NULL;
}

static int generate_data_request(size_t size)
{
	size_t i;
	if ((reqbuf = (char *)malloc(size)) == NULL)
		return -1;
	for (i = 0; i < size; ++i) {
		reqbuf[i] = 'a' + (i % 26);
	}

	rsize = size;
	return 0;
}

static int generate_post_request(const char *uri, const char* host, const char* data, size_t dsize)
{
	char buf[8192];
	// TODO: why apache close connection 5 seconds for HTTP/1.1?
	if (dsize == 0) dsize = strlen(data);
	int slen = snprintf(buf, sizeof buf, "POST %s HTTP/1.0\r\n"
			       "User-Agent: Mozilla/5.0 NetBench/1.0.00\r\n"
			       "Content-Length: %ld\r\n"
			       "Host: %s\r\n"
			       "Accept: */*\r\n"
			       "\r\n",
		 	uri, (long)dsize, host);

	buf[sizeof buf - 1] = '\0';
	rsize = slen + dsize;

	if ((reqbuf = (char *)malloc(rsize)) == NULL) {
		return -1;
	}

	memcpy(reqbuf, buf, slen);
	memcpy(reqbuf + slen, data, dsize);	

	return 0;
}

static int generate_get_request(const char *uri, const char *host)
{
	char buf[8192];
	// TODO: why apache close connection 5 seconds for HTTP/1.1?
	int slen = snprintf(buf, sizeof buf, "GET %s HTTP/1.0\r\n"
			       "User-Agent: Mozilla/5.0 NetBench/1.0.00\r\n"
			       "Host: %s\r\n"
			       "Accept: */*\r\n"
			       "\r\n",
		 	uri, host);

	buf[sizeof buf - 1] = '\0';
	if ((reqbuf = strdup(buf)) == NULL) {
		return -1;
	}

	rsize = slen;
	return 0;
}

static int load_request(const char *file)
{
	FILE *fp = fopen(file, "r");
	struct stat stbuf;
	ssize_t rlen;

	if (fp == NULL) {
		perror("fopen HTTP input file failed");
		return -1;
	}

	if (fstat(fileno(fp), &stbuf) < 0) {
		perror("fstat failed");
		goto close_fp;
	}

	rsize = stbuf.st_size;
	if ((reqbuf = (char *)malloc(rsize)) == NULL) {
		perror("malloc requst buf failed");
		goto close_fp;
	}

	if ((rlen = fread(reqbuf, 1, rsize, fp)) == (ssize_t)rsize) {
		fprintf(stderr, "stat.st_size(%ld) does not match the actual read byte (%ld).\n", (long)rsize, (long)rlen);
		goto close_fp;
	}

	fclose(fp);
	return 0;

close_fp:
	fclose(fp);
	return -1;
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
	struct connection_stat_raw *raw, all_raw;

	// get the minimum/maximum time
	double min = 3600.0 * 24 * 365 * 100, max = 0.0;
	long tot_send = 0, tot_recv = 0;
	long err_dist[E_BUTT] = { 0 };
	
	for (i = 0; i < N; ++i) {
		double dms = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_close_end);
		if (max < dms) max = dms;
		if (min > dms) min = dms;
		++err_dist[conn_times[i].err_code];
	}

	n = (int)(max + gap) / gap;
	raw = (struct connection_stat_raw *)calloc(n, sizeof(*raw));
	memset(&all_raw, 0, sizeof all_raw);

	for (i = 0; i < N; ++i) {
		double dms, tot = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_close_end);
		double threshold = tot * per;
		int bn = 0, idx = (int)tot / gap;

		++raw[idx].count;
		raw[idx].tot_total += tot;
		all_raw.tot_total += tot;

		tot_send += conn_times[i].d_send;
		tot_recv += conn_times[i].d_recv;
		
		dms = DMS(&conn_times[i].t_dns_start, &conn_times[i].t_dns_end);
		raw[idx].tot_dns += dms; all_raw.tot_dns += dms;
		if (dms >= threshold) { ++raw[idx].per_dns; ++all_raw.per_dns; ++bn; }

		dms = DMS(&conn_times[i].t_connection_start, &conn_times[i].t_connection_end);
		raw[idx].tot_connect += dms; all_raw.tot_connect += dms;
		if (dms >= threshold) { ++raw[idx].per_connect; ++all_raw.per_connect; ++bn; }

		dms = DMS(&conn_times[i].t_connection_end, &conn_times[i].t_sent);
		raw[idx].tot_send += dms; all_raw.tot_send += dms;
		if (dms >= threshold) { ++raw[idx].per_send; ++all_raw.per_send; ++bn; }

		dms = DMS(&conn_times[i].t_sent, &conn_times[i].t_first_byte);
		raw[idx].tot_first_byte += dms; all_raw.tot_first_byte += dms;
		if (dms >= threshold) { ++raw[idx].per_first_byte; ++all_raw.per_first_byte; ++bn; }

		dms = DMS(&conn_times[i].t_first_byte, &conn_times[i].t_last_byte);
		raw[idx].tot_rest_byte += dms; all_raw.tot_rest_byte += dms;
		if (dms >= threshold) { ++raw[idx].per_rest_byte; ++all_raw.per_rest_byte; ++bn; }

		dms = DMS(&conn_times[i].t_last_byte, &conn_times[i].t_close_end);
		raw[idx].tot_close += dms; all_raw.tot_close += dms;
		if (dms >= threshold) { ++raw[idx].per_close; ++all_raw.per_close; ++bn; }

		if (!bn) { ++raw[idx].per_other; ++all_raw.per_other; }
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
	fprintf(fp, "\n***** loop: %05d %04d-%02d-%02d %02d:%02d:%02d *****\n", loop,
		ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
		ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	// summary
	fprintf(fp, "\ncount(#)\tmin-time(ms)\tmax-time(ms)\tavg-total(ms)\n");
	fprintf(fp, "%ld\t%5.2f\t%5.2f\t%5.2f\n",
		    (long)N, min, max, all_raw.tot_total/N);

	fprintf(fp, "\n");
	for (i = 0; err_maps[i].err_msg != NULL; ++i) {
		fprintf(fp, "%s(#/%%)%c", err_maps[i].err_msg, 
			err_maps[i+1].err_msg == NULL ? '\n' : '\t');
	}

	for (i = 0; err_maps[i].err_msg != NULL; ++i) {
		fprintf(fp, "%ld%c", (long)err_dist[err_maps[i].err_code],
			err_maps[i+1].err_msg == NULL ? '\n' : '\t');
	}

	for (i = 0; err_maps[i].err_msg != NULL; ++i) {
		fprintf(fp, "%5.3f%%%c", 100.0 * err_dist[err_maps[i].err_code] / N,
			err_maps[i+1].err_msg == NULL ? '\n' : '\t');
	}

	fprintf(fp, "\nrange(ms)\tcount\tpercent(%%)\tavg-total\tavg-dns\tavg-connect\tavg-send\tavg-first-byte\t"
		    "avg-transfer\tavg-close\t\tper-dns(%%)\tper-connect(%%)\tper-send(%%)\tper-first-byte(%%)\t"
		    "per-transfer(%%)\tper-close(%%)\tper-other(%%)\n");
	for (i = 0; i < n; ++i) {
		if (conn_stats[loop].stats[i].count < 1) continue;
		fprintf(fp, "%05d[%ld,%ld)\t%ld\t%5.3f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t"
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

	fprintf(fp, "\n"
		    "total[%5.2f,%5.2f]\t%ld\t%5.3f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t"
		    "%5.2f\t%5.2f\t%5.2f\t\t%3.1f\t%3.1f%\t"
		    "%3.1f\t%3.1f\t%3.1f\t%3.1f\t%3.1f\n",
		    min, max, (long)N, 100.0, all_raw.tot_total/N, all_raw.tot_dns/N, all_raw.tot_connect/N,
		    all_raw.tot_send/N, all_raw.tot_first_byte/N, all_raw.tot_rest_byte/N,
		    all_raw.tot_close/N,
		    100.0*all_raw.per_dns/N, 100.0*all_raw.per_connect/N, 100.0*all_raw.per_send/N,
		    100.0*all_raw.per_first_byte/N, 100.0*all_raw.per_rest_byte/N, 100.0*all_raw.per_close/N,
		    100.0*all_raw.per_other/N);

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
		fprintf(fp, "%05d[%ld,%ld)\t%ld\t%5.3f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t"
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
	fprintf(stderr, "Usage: %s options target(its format determined by data-xchg-mode)\n"
			"  Below are the supported options:\n"
			"    -c concurrency ------- the number of concurrency. default is 1.\n"
			"    -n test-count -------- the total number of each test-loop will do. default is 10,000.\n"
			"    -t time-out ---------- the maximum time to wait for response. default is 60,000ms.\n"
			"    -i interval ---------- run test-loop every \"interval\" seconds. default is 300 seconds.\n"
			"    -C count ------------- run \"count\" times of test-loop. default is 1.\n"
			"    -P percent ----------- classify time consumer into DNS/CONN/BYTE-0/TRANS when the phase spends\n"
			"                           more than the specified \"percent\" of the total time. default is 85.\n"
			"    -g gap --------------- classify response by time step of \"gap\". default is 50ms.\n"
			"    -f threshold --------- print the statistics which takes longer than that time finally.\n"
			"                           default is 1000ms.\n"
			"    -D detailed file ----- append detailed log into this file.\n"
			"    -S statistics file --- append statistics into this file. default is \"stat.log\".\n"
			"    -O in/out-log-file --- write all input/output of each test into this file.\n"
			"  Several data communication modes:\n"
			"   (1) TCP exchange fixed-size data, which is controlled by parameters:\n"
			"        -d --------------- test connection only. Parameters below are ignored.\n"
			"        -r req-size ------ the size of request. default is determined from req-file or 1024.\n"
			"        -I req-file ------ load request from this file. without it, request is random data.\n"
			"        -s max-rsp-size -- the maximum size of response. default is 1,100,000.\n"
			"        -v min-succ-size - the minimum size of a success response. default is 0.\n"
			"        -V max-succ-size - the maximum size of a success response. default is 1,000,000.\n"
			"       Target is \"host\" \"port\".\n"
			"   (3) HTTP mode, related parameters are:\n"
			"        -m method -------- HTTP method. it supports GET, POST now. default is GET.\n"
			"        -I req-file ------ send data from this file exactly as HTTP request.\n"
			"        -s max-rsp-size -- the possible maximum size of the response.\n"
			"        -v min-succ-size - the minimum size of a success response. default is 0.\n"
			"        -V max-succ-size - the maximum size of a success response. default is 100,000,000.\n"
			"       Target is URL, like \"http://host[:port]/fcgi-bin/test.cgi\".\n"
			"\n", p);
	exit(0);
}

int main(int argc, char **argv) 
{
	struct sigaction sact;
	int ch, retval;
	char *endptr;


	while ((ch = getopt(argc, argv, "c:n:t:i:C:P:g:f:D:S:O:dr:I:s:v:V:")) != EOF) {
		switch (ch) {
		case 'c':
			tcnt = strtol(optarg, &endptr, 0);
			break;
		case 'n':
			N = strtol(optarg, &endptr, 0);
			break;
		case 't':
			timeout = strtol(optarg, &endptr, 0);
			break;
		case 'i':
			interval = strtol(optarg, &endptr, 0);
			break;
		case 'C':
			count = strtol(optarg, &endptr, 0);
			break;
		case 'P':
			per = strtol(optarg, &endptr, 0) / 100.0;
			break;
		case 'g':
			gap = strtol(optarg, &endptr, 0);
			break;
		case 'f':
			threshold = strtol(optarg, &endptr, 0);
			break;
		case 'D':
			detailed_file = optarg;
			break;
		case 'S':
			stat_file = optarg;
			break;
		case 'O':
			output_file = optarg;
			break;
		case 'd':
			xchg_data = 0;
			break;
		case 'm':
			if (strcmp(optarg, "GET") == 0) method = 'G';
			else if (strcmp(optarg, "POST") == 0) method = 'P';
			else usage(argv[0]);
		case 'I':
			input_file = optarg;
			break;
		case 's':
			rsp_buf_size = strtoul(optarg, &endptr, 0);
			break;
		case 'v':
			min_rsp_size = strtol(optarg, &endptr, 0);
			break;
		case 'V':
			max_rsp_size = strtol(optarg, &endptr, 0);
			break;
		case 'h':
			usage(argv[0]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	
	if ((argc - optind) == 2) {
		host = argv[optind];
		port = strtol(argv[optind + 1], &endptr, 0);

		if (input_file != NULL) {
			retval = load_request(input_file);
		}
		else {
			retval = generate_data_request(rsize);	
		}
	}
	else if (argc - optind == 1) {
		char h[1024], r[8192] = { '\0' };
		short p;

		if (sscanf(argv[optind], "http://%[^:/]:%d/%s", h, &p, &r[1]) >= 2) {
			// nothing
		}
		else if (sscanf(argv[optind], "http://%[^/]/%s", h, &r[1]) >= 1) {
			p = 80; // default
		}
		else {
			fprintf(stderr, "unsupported URL format. we now support http://host[:port]/...\n");
			exit(1);
		}

		r[0] = '/';
		h[sizeof h - 1] = r[sizeof r - 1] = '\0';

		uri = strdup(r);
		url = argv[optind];

		host = strdup(h);
		port = p;

		if (input_file != NULL) {
			retval = load_request(input_file);
		}
		else if (method == 'G') {
			retval = generate_get_request(uri, host);
		}
		else if (method == 'P') {
			//retval = generate_post_request(uri, host, data, dsize);
			retval = -1;
		}
		else {
			retval = -1;
		}
	}
	else {
		usage(argv[0]);
	}

	if (retval < 0) exit(5);
	
	sact.sa_handler = sigint_handler;
	sigfillset(&sact.sa_mask);
	sact.sa_flags = 0;
	sigaction(SIGINT, &sact, NULL);

	conn_times = (struct connection_times *)calloc(N, sizeof *conn_times);
	conn_stats = (struct connection_stats *)calloc(count, sizeof *conn_stats);
	rspbuf = (char *)calloc(rsp_buf_size + 16, sizeof(char)); 
	if (conn_times == NULL || conn_stats == NULL || rspbuf == NULL) {
		perror("allocate memory for conn/stat/rspbuf failed");
		exit(1);
	}

	if (detailed_file != NULL) detailed_fp = fopen(detailed_file, "a");
	stat_fp = fopen(stat_file, "a");
	if ((detailed_file != NULL && detailed_fp == NULL) || stat_fp == NULL) {
		perror("open details or statistics log file failed");
		exit(2);
	}

	if (output_file != NULL && (output_fd = open(output_file, O_WRONLY|O_APPEND|O_CREAT, 0655)) < 0) {
		fprintf(stderr, "open output-file(%s) failed: %m\n", output_file);
		exit(2);
	}

	struct timeval tv1, tv2;
	pthread_t tids[tcnt];
	long loop_idx[tcnt][2];
	int i, j;
	
	for (j = 0; j < count; ++j) {
		gettimeofday(&tv1, NULL);

		for (i = 0; i < tcnt; ++i) {
			loop_idx[i][0] = j;
			loop_idx[i][1] = i;
			if ((retval = pthread_create(&tids[i], NULL, thread_entry, (void *)&loop_idx[i])) != 0) {
				errno = retval; perror("pthread_create");
				exit(2);
			}
		}

		for (i = 0; i < tcnt; ++i) {
			pthread_join(tids[i], NULL);
		}

		if (detailed_fp) print_detailed_data(j, detailed_fp);
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
