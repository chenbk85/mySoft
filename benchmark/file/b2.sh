#
# different percentage(1~100%)
#
i=1
while test $i -le 80; do
	c=`expr 64 \* 1024 \* 1024 / 32 \* $i / 100`
	i=`expr $i + 1`

	>./tmp.log
	echo "${i}th, c=%c..."

	./file_perf_test -t 1 -b 32k -R -D -f /data/test.data -T $c -B | tee ./tmp.log
	grep "Total" ./tmp.log >>./perf.log
done
