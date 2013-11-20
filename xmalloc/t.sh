function show_all()
{
	start=$1
	end=$2
	step=$3
	count=0

	start=`expr \( $start + $step - 1 \) / $step \* $step`
	printf "/* %5d*%d */ " $step 10
	while test $start -le $end; do
		echo -n "CI($start),"
		start=`expr $start + $step`
		count=`expr $count + 1`
	done
	echo " // $count";
}

show_all 4 80 4
show_all 81 160 8
show_all 161 320 16
show_all 321 640 32
show_all 641 1280 64
show_all 1281 2560 128
show_all 2561 5120 256
show_all 5121 10240 512
show_all 10241 20480 1024
show_all 20481 40960 2048
show_all 40961 81920 4096
show_all 81921 163840 8192
show_all 163841 327680 16384
show_all 327681 655360 32768
show_all 655361 1310720 65536

