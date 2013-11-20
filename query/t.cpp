#include <string>
#include <iostream>
#include <algorithm>
#include <functional>

template <typename Tp, typename Pred = std::greater<Tp> > struct ops {
	static std::string name() { return ">"; } 
};

template <typename Tp> struct ops {
	template <std::less<Tp> >
	static std::string name() { return "<"; } 
};

int main(int argc, char **)
{
	std::cout << ops<int, std::greater<int> >::name() << std::endl;
	std::cout << ops<int>::name<std::less<int> >() << std::endl;
	return 0;
}
