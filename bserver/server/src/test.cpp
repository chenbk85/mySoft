#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <beyondy/xbs_socket.h>
#include <beyondy/xbs_io.h>

struct proto_h16_head {
	uint32_t len;
	uint16_t cmd;
	uint16_t ver;
	uint32_t syn;
	uint32_t ack;
};

struct proto_h16_res : public proto_h16_head {
	int32_t ret_;
};

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s host\n", argv[0]);
		exit(0);
	}

	const char *host = argv[1];
	int fd = beyondy::XbsClient(host, O_NONBLOCK, 30000);
	if (fd < 0) {
		perror("connect to host failed");
		exit(1);
	}
	
	char buf[8192], rbuf[8192];
	struct proto_h16_head h;
	struct proto_h16_res r;

	h.len = sizeof h + 4;
	h.cmd = 0;
	h.ver = 1;
	h.syn = 2;
	h.ack = 3;
	memcpy(buf, &h, sizeof h);
	*(int *)&buf[sizeof h] = 99;

	ssize_t wlen = beyondy::XbsWriteN(fd, buf, h.len, 10000);
	if (wlen != h.len) {
		perror("write error");
		exit(1);
	}

	ssize_t rlen = beyondy::XbsRead(fd, rbuf, sizeof rbuf, 10000);
	if (rlen != sizeof r) {
		perror("read error");
		exit(2);
	}

	memcpy(&r, rbuf, sizeof r);
	fprintf(stdout, "ret=%d\n", r.ret_);

	close(fd);
	return 0;
}
