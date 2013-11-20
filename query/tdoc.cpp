#include <iostream>
#include <sstream>
#include <string>
#include "document.h"
#include "yacc.tab.hpp"
#include "ccFlexLexer.hpp"

main(int argc, char **argv)
{
	slab_cache_init(100, 0, 0, NULL, NULL);

	fieldinfos fis;
	fix_document doc1(1, 10);
	dyn_document doc2(2);

	document_helper h1(&doc1, fis);
	document_helper h2(&doc2, fis);

	long i = 100;
	double d = 100.1234;
	char *s = "string";

	fis.add("f1", TDT_STRING);
	fis.add("f2", TDT_LONG);
	fis.add("f3", TDT_DOUBLE);

	h1.add("f1", s);
	h1.add("f2", &i);
	h1.add("f3", &d);

	h2.add("f1", s);
	h2.add("f2", &i);
	h2.add("f3", &d);

	std::cout << fis.i2n(0) << ": " << (char *)doc1.field_data(0) << std::endl;
	std::cout << fis.i2n(1) << ": " << *(long *)doc1.field_data(1) << std::endl;
	std::cout << fis.i2n(2) << ": " << *(double *)doc1.field_data(2) << std::endl;

	std::cout << fis.i2n(0) << ": " << (char *)doc2.field_data(0) << std::endl;
	std::cout << fis.i2n(1) << ": " << *(long *)doc2.field_data(1) << std::endl;
	std::cout << fis.i2n(2) << ": " << *(double *)doc2.field_data(2) << std::endl;

	//std::string qs = "n1 > 01 AND (n2 < 0x2 OR n3 <= 3) NOT n4 >= 4. AND n5 == .5 AND n6 != 6.3 AND n7: (4.e10,.5e11) OR n8:(2.3e1,3e4] OR n9:[04e4,5223.2323e4) OR n10:[2,9]";
	std::string qs = "n1 > ";
	ccFlexLexer lexer(new std::istringstream(qs), &std::cout);
	query_exp<document_helper>* exp = NULL;

	yy::query_parser parser(&lexer, &exp);
	if (parser.parse() != 0) {
		std::cerr << "parse failed" << std::endl;
	}
	else {
		std::cout << "iQuery: " << qs << std::endl;
		std::cout << "Parsed: " << exp->to_string() << std::endl;
	}

	return 0;
}

