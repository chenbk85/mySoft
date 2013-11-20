#include <iostream>
#include <string>
#include <stx/btree_map.h>
#include <sys/time.h>
#include <time.h>
#include <vector>
#include <tr1/unordered_map>

struct ts_traits
{
	static bool const selfverify = false;
	static bool const debug = false;
	static int const leafslots = 32; //64; //256; //1024;
	static int const innerslots = 32; //64;//256; //1024;
//1024: 
//256: 
//64: 
//32: 
//8:
};


typedef stx::btree_map<int, double, std::less<int>, ts_traits> tstree_t;
typedef struct tsinfo {
	char secid[10];
	tstree_t daily;
} tsinfo_t;

#undef VEC

#ifdef VEC
static std::vector<tsinfo_t*> tss;
#else

struct secid {
	char id[10];
};

struct secid_hash {
};

struct secid_eq {
	bool operator()(const struct secid& x, const struct secid& y) const {
		return memcmp(x.id, y.id, 10) == 0;
	}
};

static std::tr1::unordered_map<secid, tstree_t*, secid_hash, secid_eq, tstree_t*> tss;
#endif

static int tss_load(const char* file)
{
	struct timeval t1, t2;
	FILE *fp = fopen(file, "r");
	char line[8192], *pid = "";
	tsinfo_t *last = NULL;
	int count = 0;

	if (fp == NULL) { perror("fopen failed"); return -1; }

	gettimeofday(&t1, NULL);
	while (fgets(line, sizeof(line), fp) != NULL) {
		// F000004687,2006-03-31 00:00:00,-657128000.00000
		char *id = line;
		char *dptr = NULL, *vptr = NULL;	
		dptr = strchr(line, ',');
		if (dptr != NULL) { *dptr++ = 0; vptr = strchr(dptr, ','); }
		if (dptr != NULL && vptr != NULL) {
			*vptr++ =  0;
#if 1
			if (strncmp(pid, id, 10) != 0) {
				if (last != NULL) {
					++count;
					tss.push_back(last);
//fprintf(stderr, "secid: %.10s, count: %ld\n", last->secid, (long)last->daily.size());
				}

				last = new tsinfo_t();
				memcpy(last->secid, id, 10);
				pid = last->secid;
			}

			struct tm tbuf;
			time_t ts;
			int day;
#if 0
			strptime(dptr, "%Y-%m-%d", &tbuf);
			ts = mktime(&tbuf);
			day = ts / (24*3600);
#else
			int y = 1, m = 1, d = 1;
			sscanf(dptr, "%d-%d-%d", &y, &m, &d);
			day = y * 10000 + m * 100 + d;
#endif
			double val = strtod(vptr, NULL);
			last->daily.insert(day, val);
#else
			tss[std::string(id)].insert(day, val);	
#endif
		}
	}

	if (last != NULL) {
		tss.push_back(last);
		++count;
	}

	fclose(fp);

	gettimeofday(&t2, NULL);
	double ms = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000;
	fprintf(stderr, "load %s, count=%d, total=%ld cost %.3fms\n", file, count, (long)tss.size(), ms);

	return 0;
}

static int tss_search(int dstart, int dend, double vstart, double vend)
{
	int count = 0;
	for (std::vector<tsinfo_t*>::iterator iter = tss.begin(), end = tss.end();
			iter != end;
				++iter) {
		bool find = true;
		tstree_t::iterator i1 = (*iter)->daily.lower_bound(dstart);
		tstree_t::iterator i2 = (*iter)->daily.upper_bound(dend);

		for (/* ** */; i1 != i2; ++i1) {
			if (i1->second <= vstart || i1->second >= vend) {
				find = false;
				break;
			}
		}

		if (find) ++count;
	}

	return count;
}

static void usage(const char* p)
{
	fprintf(stderr, "Usage: %s -d start-day -e end-day -v val-from -E val-end file [...]\n", p);
	exit(1);
}

int main(int argc, char **argv)
{
	int ch;
	int dstart = 0, dend = 1000;
	double vstart = 0.0, vend = 10000.0;

	while ((ch = getopt(argc, argv, "d:e:v:E:h")) != EOF) {
		switch (ch) {
		case 'd':
			dstart = atoi(optarg);
			break;
		case 'e':
			dend = atoi(optarg);
			break;
		case 'v':
			vstart = strtod(optarg, NULL);
			break;
		case 'V':
			vend = strtod(optarg, NULL);
			break;
		case 'h':
			usage(argv[0]);
			/* exit */
		}
	}

	for (int i = optind; i < argc; ++i) {
		tss_load(argv[i]);
	}

	struct timeval t1, t2;

	do {
		gettimeofday(&t1, NULL);
		int matched = tss_search(dstart, dend, vstart, vend);
		gettimeofday(&t2, NULL);
		double ms = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000;
		fprintf(stderr, "search(%d, %d, %.3f, %.3f) cost %.3fms, matched=%d\n", \
			dstart, dend, vstart, vend, ms, matched);

		fprintf(stderr, "input day-start day-end val-start val-end: ");
		fflush(NULL);

		int v1, v2;
		if (scanf("%d %d %d %d", &dstart, &dend, &v1, &v2) != 4) {
			break;
		}

		vstart = v1;
		vend = v2;
	} while (1);

	return 0;
}
