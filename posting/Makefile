TARGET = index.slab index.new index.old index.v4 index.map testslab
#TARGET = index.new index.old index.v4 index.map testslab
OBJS   = posting.new.o

INC    = -I ../svn/trunk/common/include `pkg-config --cflags libxml-2.0` -I ~/opensource/stx-btree-0.8.3/include/ -I .
LIBS   = -L ../svn/trunk/common/lib -lcommon `pkg-config --libs libxml-2.0`
CFLAGS = -O3 -g
#CFLAGS = -g

all: $(TARGET)

index.new: posting.new.o
	g++ -o $@ $^ $(LIBS)
index.old: posting.old.o
	g++ -o $@ $^ $(LIBS)
index.v4: posting.v4.o
	g++ -o $@ $^ $(LIBS)
index.map: posting.map.o
	g++ -o $@ $^ $(LIBS)
index.slab: posting.slab.o
	g++ -o $@ $^ $(LIBS)
.cpp.o:
	g++ -c $^ $(CFLAGS) $(INC)
.c.o:
	gcc -c $^ $(CFLAGS) $(INC)
testslab: testslab.cpp
	g++ -g -O3 -o testslab testslab.cpp $(INC) $(LIBS)
tss: tss.cpp
	g++ -g -O3 -o tss tss.cpp $(INC) $(LIBS)
clean:
	rm -rf $(TARGET) $(OBJS) testslab *.o
