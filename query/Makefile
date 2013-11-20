ALL=tdoc
%.yy.cpp: %.l
	flex -o$@ -+ -f $^
%.tab.cpp: %.ypp
	bison $^

CC=gcc
CPP=g++
CFLAGS = -I . -g
CPPFLAGS = -I . -g

OBJS = tdoc.o document.o slab_cache.o rbtree.o yacc.tab.o lex.yy.o

tdoc: $(OBJS)
	echo "RM=$(RM)"
	g++ -o tdoc $(OBJS)
clean:
	rm -rf *.o lex.yy.cpp yacc.tab.cpp tdoc
