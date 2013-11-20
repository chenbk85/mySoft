#
# different block size(4K ~ 512K)
#

# random-mode
for i in 4 8 16 32 64 128 256 512; do
	c=`expr 6 \* 1024 \* 1024 / $i`
	#c=`expr 50 \* 1024 / $i`
	./file_perf_test -t 1 -b ${i}k -R -D -f /data/test.data -T $c | grep -- "Total" >>b.log
done

echo

# binlog-mode
for i in 4 8 16 32 64 128 256 512; do
	c=`expr 6 \* 1024 \* 1024 / $i`
	#c=`expr 50 \* 1024 / $i`
	./file_perf_test -t 1 -b ${i}k -R -D -f /data/test.data -T $c -B | grep -- "Total" >>b.log
done


