#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define USIZE 10*1024*1024

static int get_mem_swap_size(uint64_t* msize, uint64_t* wsize)
{
	FILE* fp = popen("grep -P \"MemTotal|SwapTotal\" /proc/meminfo", "r");
	char line[1024];

	if (fp == NULL) {
		fprintf(stderr, "popen(grep -P \"MemTotal|SwapTotal\" /proc/meminfo) failed: %m\n");
		return -1;	
	}

	while (fgets(line, sizeof line, fp) != NULL) {
		char name[512], unit[48];
		unsigned long long num;

		if (sscanf(line, "%s%llu%s", name, &num, unit) != 3)
			break;

		uint64_t *sptr;
		if (strcasecmp(name, "MemTotal:") == 0) {
			*msize = num;
			sptr = msize;
		}
		else if (strcasecmp(name, "SwapTotal:") == 0) {
			*wsize = num;
			sptr = wsize;
		}
		else {
			fprintf(stderr, "unknown field: %s\n", name);
			continue;
		}

		if (strcasecmp(unit, "kB") == 0) {
			*sptr *= 1024;
		}
		else if (strcasecmp(unit, "mB") == 0) {
			*sptr *= 1024 * 1024;
		}
		else if (strcasecmp(unit, "gB") == 0) {
			*sptr *= 1024 * 1024 * 1023;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	uint64_t msize = 5 * 1024ULL * 1024ULL * 1024ULL;
	uint64_t wsize = msize;
	uint64_t msum;

	get_mem_swap_size(&msize, &wsize);

	fprintf(stdout, " MemTotal: %.3f\n", msize / 1024.0 / 1024.0 / 1024.0);
	fprintf(stdout, "SwapTotal: %.3f\n", wsize / 1024.0 / 1024.0 / 1024.0);
	
        for (msum = 0; msum < (msize + wsize / 10); msum += USIZE) {
                char *p = (char *)malloc(USIZE);
                if (p == NULL)
                        break;
                memset(p, 0, USIZE);
        }

        return 0;
}
