#      exchange sec-type tz      open-time symbos
tabs=( 126      1        "-0600" "570"     "IBM GOOG MSFT"
       126      10       "-0600" "570"     "SPX ID1 ID2"
  )

FRATE=70000 #70.000
errcnt=0
errmsg=""

s=`TZ=GMT date +%s`
i=0
enum=${#tabs[*]}
while test $i -lt $enum; do
	j=`expr $i + 2`
	tz=${tabs[$j]}
	sign=`echo $tz | awk '{ print substr($1,1,1) }'`
	h=`echo $tz | awk '{ print substr($1,2,2) }'`
	m=`echo $tz | awk '{ print substr($1,4,2) }'`
	es=`expr $s $sign \( $h \* 3600 + $m \* 60 \)`
	#ed=`TZ=GMT date -d @$es +%Y-%m-%d`
	ed=`TZ=GMT date -d @$es`
	em=`expr $es % 86400 / 60`

	echo "s=$s j=$j, tz=$tz, sign=$sign, hour=$h, min=$m, es=$es ed=$ed, em=$em, s-es=`expr $s - $es`"

	j=`expr $i + 3`
	c=${tabs[$j]}
	c1=`expr $c + 2`
	c2=`expr $c + 10`
#	if test $em -eq $c1 -o $em -eq $c2; then
	if true; then
		j=`expr $i + 4`
		n=0
		t=${tabs[$(($i+4))]}
		succ=0
		fail=0
		eval "symbols=($t)"
		snum=${#symbols[*]}
		for s2 in ${symbols[*]}; do
			echo "s=$s2"
			if curl URL 2>/dev/null | grep $ed >/dev/null 2>&1; then
				#OK
				succ=$((succ+1))
			else
				#Fail
				fail=$((fail+1))
			fi
		done

		frate=`expr $fail \* 1000 \/ $snum`
		if test $frate -gt $FRATE; then
			errcnt=$((errcnt+1))
			errmsg="$errmsg $fail/$tcnt(xx%) in ($t) failed. xxxxx"
		fi
	fi

	i=$((i+5))
done

if test $errcnt -ge 0; then
	echo "FATAL|$errcnt|$errmsg";
else
	echo "OK|0|OK";
fi
