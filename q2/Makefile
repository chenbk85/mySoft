query: lex.yy.cpp yacc.tab.cpp
	g++ -o $@ $*
%.yy.cpp: %.l
	flex -+ -o $@ $^
%.tab.cpp: %.ypp
	bison -o $@ $^

tdoc: tdoc.cpp document.cpp
	g++ -o tdoc.cpp document.cpp  -I ~/svn/trunk/common/include
