INC = -I ../../common/include -I ../../share/include -I /usr/include/libxml2
LIB = -L ../../common/lib -lcommon -lxml2 -lz -lm -L /data/mstar/storage/depends -llog4cpp -lpthread

CXXFLAGS = -Wall -g -fPIC
LDFLAGS  =

DEST = ../bin/bserver
OBJS = server.o acceptor.o message_block.o protocol.o schedule.o \
       connection.o event_manager.o proto_h16.o sched_mt.o

ECHO = ../lib/echo.so
EOBJ = proc_echo.o message_block.o

TEST = ../bin/echo_test
TOBJ = echo_test.o

all: $(DEST) $(ECHO) $(TEST)

$(DEST): $(OBJS)
	g++ -o $@ $(OBJS) $(LIB)

$(ECHO): $(EOBJ)
	g++ -shared -o $@ $(EOBJ) $(LIB)

$(TEST): $(TOBJ)
	g++ -o $@ $(TOBJ) $(LIB)

.cpp.o:
	g++ -fPIC -c -o $@ $(INC) $(CXXFLAGS) $<
.c.o:
	gcc -c -o $@ $(INC) $(CXXFLAGS) $<

clean:
	rm -f $(OBJS) *~ *.s *.ii *.i
