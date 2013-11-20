#include <stx/btree.h>
#include <stx/btree_map.h>
#include <map>
#include <string>
#include <malloc.h>
#include <sys/time.h>

class Key {
public:
	Key() {}
	Key(int i)
	{
		int j = 0;
		do {
			data_[j++] = '0' + (i % 10);
			i = i / 10;
		} while (i > 0);

		for (/* */; j < 10; ++j)
			data_[j] = 0;
	}

	bool operator<(const Key& y) const
	{
		for (int i = 0; i < sizeof(data_); ++i) {
			if (data_[i] < y.data_[i])
				return true;
			else if (data_[i] > y.data_[i])
				return false;
		}

		return false;
	}

	const char* c_str() const { return data_; }
private:
	char data_[10];
};

static void test_map(int count)
{
	std::map<Key, long> m;
	for (int i = 0; i < count; ++i) {
		Key k(i);
		m[k] = i;
	}

	fprintf(stderr, "map-size: %ld\n", (long)m.size());
/*
	for (std::map<Key, long>::iterator iter = m.begin(); iter != m.end(); ++iter) {
		fprintf(stderr, "k: %s ==> %ld\n", iter->first.c_str(), iter->second);
	}
**/
	int j = 1147;
	Key k(j);
	std::map<Key, long>::iterator iter = m.find(k);
	if (iter != m.end()) {
		fprintf(stderr, "find: %ld\n", iter->second);
	}
}

static void test_btree(int count)
{
	stx::btree_map<Key, long> m;
	for (int i = 0; i < count; ++i) {
		Key k(i);
		m.insert(k, i);
	}

	fprintf(stderr, "map-size: %ld\n", (long)m.size());

	int j = 1147;
	Key k(j);
	stx::btree_map<Key, long>::iterator iter = m.find(k);
	if (iter != m.end()) {
		fprintf(stderr, "find: %ld\n", iter->second);
	}
}

static void show_minfo()
{
	struct mallinfo mi = mallinfo();
	fprintf(stderr, "mallinfo: arena=%d ordblks=%d smblks=%d hblks=%d hblkhd=%d usmblks=%d fsmblks=%d uordblks=%d fordblks=%d keepcost=%d\n", mi.arena, mi.ordblks, mi.smblks, mi.hblks, mi.hblkhd, mi.usmblks, mi.fsmblks, mi.uordblks, mi.fordblks, mi.keepcost);
}

int main(int argc, char **argv)
{
	struct timeval t1, t2;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s [--map|--btree] count\n", argv[0]);
		exit(1);
	}

	gettimeofday(&t1, NULL);
	if (std::string("--map") == argv[1]) {
		test_map(atoi(argv[2]));
	}
	else {
		test_btree(atoi(argv[2]));
	}

	gettimeofday(&t2, NULL);
	double ms = (t2.tv_sec - t1.tv_sec) / 1000.0 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	fprintf(stderr, "insert into map, cost: %.3fms\n", ms);
	show_minfo();

	sleep(30);
	return 0;
}
