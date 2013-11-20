( iostat -xm 30 & while true; do date; sleep 30; done ) >iostat.log 2>&1 &
iopid=$!

( vmstat 30 & while true; do date; sleep 30; done ) >vmstat.log 2>&1 &
vmpid=$!

FILE="/data/test.data"
PDISK="/dev/mapper/vg_data-lv_data"
BC=1047586
BS=65536

for i in 0 1 2; do
	date
	echo "do free mem for dd input[$i]..."
	./freemem

	date
	echo "do dd (writing) [$i]..."
	dd if=/dev/zero of=$FILE bs=$BS count=$BC

	date
	echo "do free mem for dd output[$i]..."
	./freemem

	date
	echo "do dd (reading) [$i]..."
	dd of=/dev/null if=$FILE bs=$BS count=$BC
done

date
echo "do free mem for file-random I/O..."
./freemem

TC=`expr $BC \* $BC / 4096`
BS=4096

date
echo "do file-random I/O ..."
./file_perf_test -r 0 -t 2 -b $BS -R -D -T $TC -f $FILE 

for i in 0 1 2 3; do
	date
	echo "do hdparam -tT[$i] ..."
	/sbin/hdparm -tT $PDISK
done

sleep 30
kill $iopid $vmpid
sleep 1
kill $iopid $vmpid
sleep 1
kill -9 $iopid $vmpid

