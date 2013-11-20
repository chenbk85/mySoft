#include <iostream>
#include <map>
#include <string>
#include <tr1/functional>
#include <cstdio>
#include <cstdlib>

class const_node {
public:
	const_node(std::string ip) : _ip(ip) {}
	const_node(const const_node& n) : _ip(n._ip) {}
public:
	const std::string& ip() const { return _ip; }
	std::string& ip() { return _ip; }
private:
	std::string _ip;
};

class const_hash {
public:
	const_hash(int vn) : _vn(vn) {}
public:
	void add_node(const const_node& n) {
		const_node* pn = new const_node(n);
		for (int i = 0; i < _vn; ++i) {
			_hmap[hash(n.ip(), i)] = pn;
		}
	}

	void del_node(const const_node& n) {
		const_node *pn = _hmap[hash(n.ip(), 0)];
		for (int i = 0; i < _vn; ++i) {
			_hmap.erase(hash(n.ip(), i));
		}

		delete pn;
	}

	std::string get_pos(int s) {
		if (s < _hmap.begin()->first) s += _hmap.begin()->first;
		int i = hash(s);
fprintf(stdout, "%d -> %d\n", s, i);
		std::map<int, const_node*>::iterator iter = _hmap.lower_bound(i);
		if (iter == _hmap.end()) iter = _hmap.begin();
		return iter->second->ip();	
	}

	void print() const {
		fprintf(stdout, "vn = %d, virtual node: %d\n", _vn, _hmap.size());
		for (std::map<int, const_node*>::const_iterator iter = _hmap.begin(); iter != _hmap.end(); ++iter) {
			fprintf(stdout, " vn-%04d -> %s\n", iter->first, iter->second->ip().c_str());
		}
	}

private:
	int hash(int i) {
		return i % 9973;
	}

	int crc32(const std::string& s) {
		std::tr1::hash<const char*> hf;
		return hf(s.c_str());	
	}

	int hash(const std::string& s, int i) {
		char buf[128];
		snprintf(buf, sizeof buf, "%d", i);
		int c = crc32(s + buf);
		return hash(c);	
	}
private:
	int _vn;
	std::map<int, const_node*> _hmap;
};

int main(int argc, char** argv)
{
	if (argc < 3 || argc == 1 && strcmp(argv[1], "-h") == 0) {
		fprintf(stderr, "Usage: %s -h | virt-num ip ...\n", argv[0]);
		exit(1);
	}

	int vn = strtol(argv[1], NULL, 0);
	const_hash ch(vn);

	for (int i = 2; i < argc; ++i) {
		ch.add_node(const_node(argv[i]));
	}

	ch.print();

	for (int i = 0; i < 100; ++i) {
		std::string ip = ch.get_pos(i);
		fprintf(stdout, "%04d --> %s\n", i, ip.c_str());
	}

	return 0;
}
