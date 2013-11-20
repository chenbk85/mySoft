TARGET = libxbsm.so
OBJS   = xbs_malloc.o slab_cache.o rbtree.o

#CFLAGS = -fPIC -I . -g -pg
CFLAGS = -fPIC -I . -O3

$(TARGET): $(OBJS)
	gcc -shared -o $@ $(OBJS)
main: main.o slab_cache.o rbtree.o
	g++ -O3 -o $@ $^ -lpthread
clean:
	rm -rf *.o main
