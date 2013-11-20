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
#include <unistd.h>
#include <signal.h>

#define TV_DELTA_MS(t1, t2)	( ((t2)->tv_sec  - (t1)->tv_sec ) * 1000 + \
			          ((t2)->tv_usec - (t1)->tv_usec) / 1000 )

static char buffer[8 * 1024 * 1024];
static int running = 1;

static void sigint_handler(int sig)
{
	running = 0;
}

static void dump_sock_option(int fd)
{
	int bsize = 0;
	socklen_t blen = sizeof(int);
	int retval;

	if ((retval = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&bsize, &blen)) < 0) {
		perror("getsockopt(SO_SNDBUF");
	}
	else {
		printf("SO_SNDBUF default: %d\n", bsize);
	}
	if ((retval = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&bsize, &blen)) < 0) {
		perror("getsockopt(SO_RCVBUF)");
	}
	else {
		printf("SO_RCVBUF default: %d\n", bsize);
	}
}

static void usage(const void *exe)
{
	fprintf(stderr, "Usage: %s [ -r] ip port\n", exe);
	exit(0);
}

int main(int argc, char **argv) 
{
	struct sockaddr_in addr;
	struct sigaction sact;

	fd_set rfds, wfds;
	short port = 5000;
	int fd;
	int ch, response = 0, retval;
	char *endptr;

	unsigned long long send_byte = 0, recv_type = 0;
	unsigned long long send_count = 0, recv_count = 0;

	int rlen, slen;
	struct timeval tv1, tv2, to;
	unsigned long ms;

	while ((ch = getopt(argc, argv, "rh")) != EOF) {
		switch (ch) {
		case 'r':
			response = 1;
			break;
		case 'h':
			usage(argv[0]);
			break;
		default:
			usage(argv[0]);
			break;
		}
	}
	
	if ((argc - optind) != 2) 
		usage(argv[0]);
	
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(strtol(argv[optind + 1], NULL, 0));
	addr.sin_addr.s_addr = inet_addr(argv[optind]);

	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	dump_sock_option(fd);

	if ((retval = connect(fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
		perror("connect");
		exit(2);
	}

	dump_sock_option(fd);

	sact.sa_handler = sigint_handler;
	sigfillset(&sact.sa_mask);
	sact.sa_flags = 0;

	printf("Now test network rate with %s, press CTRL+C to STOP ....\n", inet_ntoa(addr.sin_addr));
	fflush(NULL);

	sigaction(SIGINT, &sact, NULL);

	gettimeofday(&tv1, NULL);

	while (running) {

		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(fd, &rfds);
		FD_SET(fd, &wfds);

		to.tv_sec = 0;
		to.tv_usec = 10000;

		if ((retval = select(fd + 1, &rfds, &wfds, NULL, &to)) < 0) {
			if (errno != EINTR)
				break;
		}

		if (FD_ISSET(fd, &wfds)) {
			if ((slen = send(fd, (void *)buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
				send_byte += slen;
				send_count++;
			}
			else if (slen < 0 && !(errno == EAGAIN || errno == EINTR)) {
				break;
			}
		}

		if (response && FD_ISSET(fd, &rfds)) {
			if ((rlen = recv(fd, (void *)buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
				recv_type += rlen;
				recv_count++;
			}
			else if (rlen < 0 && !(errno == EAGAIN || errno == EINTR)) {
				break;
			}
		}
	}

	gettimeofday(&tv2, NULL);

	dump_sock_option(fd);

	ms = TV_DELTA_MS(&tv1, &tv2);
	if (ms == 0) ms = 1;

	printf("test in %lums\n", ms);

	if (send_count == 0) send_count = 1;
	printf("send: %llubytes, rate=%llukb/s, count=%llu, %llub/p\n", 
		send_byte, send_byte / ms, send_count, send_byte / send_count );

	if (recv_count == 0) recv_count = 1;
	printf("recv: %llubytes, rate=%llukb/s, count=%llu, %llub/p\n",
		recv_type, recv_type / ms, recv_count, recv_type / recv_count );
	
	return 0;
}
