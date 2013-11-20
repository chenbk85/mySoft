thread=10;
i=0
for file in *; do
	( d="$$: `date`"; if test -f $file; then cat $file | grep "do" >/tmp/file_$i.txt; wc -l /tmp/file_$i.txt; echo $d; echo;fi; ) &
	i=$((i+1))
	if test $i -eq $thread; then
		echo "wait for all";
		wait;
		rm /tmp/file_?.txt
		i=0
	fi
done
