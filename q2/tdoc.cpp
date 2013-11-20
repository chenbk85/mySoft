#include <iostream>

#include "document.h"

main(int argc, char **argv)
{
	fieldinfos fis;
	fix_document doc1(10);
	dyn_document doc2;

	document_helper h1(&doc1, fis);
	document_helper h2(&doc2, fis);

	return 0;
}

