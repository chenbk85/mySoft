#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define TV_DELTA_MS(v1, v2)  ( ((v2)->tv_sec  - (v1)->tv_sec ) * 1000 + \
			       ((v1)->tv_usec - (v1)->tv_usec) / 1000 )

static void usage(const void *exe)
{
	fprintf(stderr, "Usage: %s -p port [ -s] [ -r] \n", exe);
	exit(0);
}

static int status = 0;
static int response = 0;
static char buffer[8 * 1024 * 1024];

static void *child_handler(void *fd_void)
{
	fd_set rfds, wfds;
	struct timeval tv1, tv2, to;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int fd = (long)fd_void;

	unsigned long long send_byte = 0, recv_byte = 0;
	unsigned long long send_count = 0, recv_count = 0;

	int len, retval;
	unsigned long ms;
	
	if ((retval = getpeername(fd, (struct sockaddr *)&addr, &addr_len)) < 0) {
		perror("getpeername");
		goto out;
	}

	printf("get connection from %s.%d ...\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	fflush(NULL);

	gettimeofday(&tv1, NULL);

	while ( 1 ) {

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

		if (FD_ISSET(fd, &rfds)) {
			if ((len = recv(fd, (void *)buffer, sizeof(buffer), MSG_DONTWAIT)) == 0) {
				break;
			}
			else if (len < 0 && !(errno == EAGAIN || errno == EINTR)) {
				break;
			}
			else if (len > 0) {
				recv_byte += len, recv_count++;
			}
		}

		if (response && FD_ISSET(fd, &wfds)) {
			if ((len = send(fd, (void *)buffer, sizeof(buffer), MSG_DONTWAIT)) < 1) {
				if (!(errno == EAGAIN || errno == EINTR))
					break;
			}
			else {
				send_byte += len, send_count++;
			}
		}
	}

	gettimeofday(&tv2, NULL);
	ms = TV_DELTA_MS(&tv1, &tv2);
	if (ms == 0) ms = 1;

	printf("cmmunicate with %s.%d, cost: %lums\n", 
		inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), ms);

	if (send_count == 0) send_count = 1;
	printf("send %llu bytes, rate: %llukb/s, count: %llu, average %llub/p\n",
		send_byte, send_byte / ms, send_count, send_byte / send_count );

	if (recv_count == 0) recv_count = 1;
	printf("recv %llu bytes, rate: %llukb/s, count: %llu, average %llub/p\n",
		recv_byte, recv_byte / ms, recv_count, recv_byte / recv_count );

	printf("\n");
	fflush(NULL);

out:
	close(fd);
	return NULL;
}

int main(int argc, char **argv) 
{
	struct sockaddr_in addr;
	pthread_t tid;
	short port = 5000;
	int sfd, cfd, ch, retval;

	while ((ch = getopt(argc, argv, "p:srh")) != EOF) {
		switch (ch) {
		case 'p':
			port = strtol(optarg, NULL, 0);
			break;
		case 's':
			status = 1;
			break;
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
	
	if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if ((retval = bind(sfd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
		perror("bind");
		exit(2);
	}

	if ((retval = listen(sfd, 5)) < 0) {
		perror("listen");
		exit(3);
	}

	while ((cfd = accept(sfd, NULL, 0)) > 0) {
		if ((retval = pthread_create(&tid, NULL, (void *(*)(void *))child_handler, (void *)(long)cfd)) != 0) {
			errno = retval;
			perror("pthread_create");
		}
	}

	return 0;
}

