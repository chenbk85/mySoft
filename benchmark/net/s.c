#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char h[1024], r[1024] = { 0, 1,  };
	int p = -1;
	
	if (sscanf(argv[1], "http://%[^:/]:%d/%s", h, &p, r) >= 2) {
		// nothing
	}
	else if (sscanf(argv[1], "http://%[^:/]/%s", h, r) == 1) {
		p = 80; // default
	}
	else {
		fprintf(stderr, "unsupported URL format. we now support http://host[:port]/...\n");
		exit(1);
	}

	fprintf(stdout, "host:[%s], port:[%d], uri:[%s]\n", h, p, r);
	return 0;
}
