INC = -I ../include -I /usr/include/libxml2
LIB = -lxml2 -lz -lm

CXXFLAGS = -Wall -g -fPIC -O3
LDFLAGS  =

DEST = ../lib/libcommon.a
OBJS = log.o pid_file.o xmlconfig.o xbs_naddr.o xbs_io.o xbs_socket.o nam2val.o md5.o slab_cache.o rbtree.o

all: $(DEST)

$(DEST): $(OBJS)
	ar cru $@ $(OBJS)

.cpp.o:
	g++ -fPIC -c -o $@ $(INC) $(CXXFLAGS) $<
.c.o:
	gcc -c -o $@ $(INC) $(CXXFLAGS) $<

clean:
	rm -f $(OBJS) *~ *.s *.ii *.i
