function get_ip()
{
	d=$1;
	ping -W 1 -c 1 $d >/tmp/t.txt 2>&1
	if grep "^PING" /tmp/t.txt | grep "bytes of data" >/dev/null 2>&1; then
		cat /tmp/t.txt | head -n 1 | sed -e 's/^[^(]\+(//' -e 's/).*$//'
	else
		echo "N/A"
	fi
}

for d in s3 s3do awds3 s3sigma s3rbc; do
	gtm_ip=`get_ip ${d}`
	dallas_ip=`get_ip ${d}-dallas`
	b37_ip=`get_ip ${d}-b37`

	if test "${gtm_ip}" = "${dallas_ip}"; then
		gtm_eq="${d}-dallas";
	elif test "${gtm_ip}" = "${b37_ip}"; then
		gtm_eq="${d}-b37";
	else
		gtm_eq="N/A";
	fi
	echo -e "$d(${gtm_ip})=${gtm_eq}\t${d}-dallas(${dallas_ip})\t${d}-b37(${b37_ip})"
done
