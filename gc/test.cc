#include <iostream>
#include <string>
#include "object.h"

class foo: public object {
public:
	foo(const char *s) : _name(s) { std::cout << "foo(" + _name + ")" << std::endl; }
	virtual ~foo() { std::cout << "~foo(): " + _name << std::endl; }
private:
	std::string _name;
};

class bar : public object {
public:
	object_ptr n;
};

typedef _object_ptr<foo> foo_ptr;
typedef _object_ptr<bar> bar_ptr;

int main(int argc, char **argv)
{
#define P(p) std::cout << p << std::endl;
P("p1()");
	foo_ptr p1(new foo("1"));
P("define local p2 = p1")
	{
		object_ptr p2;
		p2 = p1;
	}
P("define P3=p1");
	object_ptr p3 = p1;
p1.debug("p1 should 2");
P("define P4(new)");
	object_ptr p4(new foo("2"));
P("P4<--P3");
	p4 = p3;
p1.debug("p1 should 3");
P("define P4(p4)");
	object_ptr p5(p4);
p1.debug("p1 should 4");
p5.debug("p5");
p4.debug("p4");
p3.debug("p3");
p1.debug("p1");
	bar_ptr p6(new bar);
std::cout << "===" << std::endl;
	p6->n = p5;
	p6->n.debug("after assign");
std::cout << "===" << std::endl;
P("before exit");
	return 0;
}
