#include <iostream>
#include <string>

#include "shared_ptr.hpp"

static int i = 0;
class obj {
public:
	obj() { no = i++; fprintf(stdout, "obj(): %d\n", no); }
	obj(const obj& o) { no = i++; fprintf(stdout, "obj(obj: %d): %d\n", o.no, no); }
	~obj() { fprintf(stdout, "~obj(): %d\n", no); }
	obj& operator=(const obj& o) { no = i++; fprintf(stdout, "obj: %d <= obj: %d\n", no, o.no); }
private:
	int no;
};

int main(int argc, char **argv)
{
	shared_ptr<obj> a1;
	shared_ptr<obj> a2(new obj);
	shared_ptr<obj> a3(a2); //new obj(**a2));

	std::cout << a1 << std::endl;
	std::cout << a2 << std::endl;
	std::cout << a3 << std::endl;

	return 0;
}
