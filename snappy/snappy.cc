#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>
#include "snappy.h"

int file_read(FILE* ifp, std::vector<char>& data)
{
	char buf[64*1024];
	ssize_t rlen;

	while ((rlen = fread(buf, 1, sizeof buf, ifp)) > 0) {
		data.insert(data.end(), buf, buf + rlen);
	}

	if (rlen == 0) return 0;
	if (data.size() < 1) return -1;
	return 0;
}

static int do_compress(FILE* ifp, FILE* ofp)
{
	std::vector<char> data;
	if (file_read(ifp, data) < 0) {
		fprintf(stderr, "fstat input failed: %m\n");
		return -1;
	}
	
	char* out = new char[snappy::MaxCompressedLength(data.size())];
	size_t olen, wlen;

	snappy::RawCompress(data.data(), data.size(), out, &olen);
	if ((wlen = fwrite(out, 1, olen, ofp)) != olen) {
		fprintf(stderr, "fwrite output failed: %m (%ld out of %ld).\n", (long)wlen, (long)olen);
		return -1;
	}

	delete [] out;

	return 0;
}

static int do_uncompress(FILE* ifp, FILE* ofp)
{
	std::vector<char> data;
	if (file_read(ifp, data) < 0) {
		fprintf(stderr, "fstat input failed: %m\n");
		return -1;
	}
	
	size_t olen, wlen;
	char* out;

	if (!snappy::GetUncompressedLength(data.data(), data.size(), &olen)) {
		fprintf(stderr, "failed to get uncompressed lenth: %m.\n");
		return -1;
	}

	out = new char[olen];
	if (!snappy::RawUncompress(data.data(), data.size(), out)) {
		fprintf(stderr, "snapp-uncompress failed: %m.\n");
		return -1;
	}

	if ((wlen = fwrite(out, 1, olen, ofp)) != olen) {
		fprintf(stderr, "fwrite output failed: %m (%ld out of %ld).\n", (long)wlen, (long)olen);
		return -1;
	}

	delete [] out;

	return 0;
}

static void usage(const char* p)
{
	fprintf(stderr, "Usage: %s -d | -c [ input-file [ out-file ] ]\n", p);
	exit(0);
}

int main(int argc, char **argv)
{
	FILE *ifp = stdin, *ofp = stdout;
	int ch, action = 'c';

	while ((ch = getopt(argc, argv, "dc")) != EOF) {
		switch (ch) {
		case 'd':
		case 'c':
			action = ch;
			break;
		default:
			fprintf(stderr, "unkown flag: %c.\n", ch);
			usage(argv[0]);
			// no here
		}
	}

	if ((argc - optind) >= 1) {
		if ((ifp = fopen(argv[optind], "r")) == NULL) {
			fprintf(stderr, "open(%s) for input failed: %m.\n", argv[optind]);
			exit(1);
		}

		if ((argc - optind) == 2) {
			if ((ofp = fopen(argv[optind + 1], "w")) == NULL) {
				fprintf(stderr, "open(%s) for output failed: %m.\n", argv[optind + 1]);
				exit(2);
			}
		}
		else if ((argc - optind) > 2) {
			usage(argv[0]);
		}
	}

	if (action == 'c') {
		do_compress(ifp, ofp);
	}
	else {
		do_uncompress(ifp, ofp);
	}

	if (ifp != stdin) fclose(ifp);
	if (ofp != stdout) fclose(ofp);

	return 0;
}
