inst=(s3 s3sigma s3rbc)
for i in $inst; do
	ping $i
	ping ${i}-b37
	ping ${i}-dallas
done
