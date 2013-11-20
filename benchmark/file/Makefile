all: file_perf_test freemem

file_perf_test: file_perf_test.cpp
	#g++ -g -o $@ -D_FILE_OFFSET_BITS=64 $< -lpthread
	g++ -O3 -o $@ -D_FILE_OFFSET_BITS=64 $< -lpthread
freemem: freemem.c
	gcc -O3 -o $@ $<

clean:
	rm -rf file_perf_test freemem *~
