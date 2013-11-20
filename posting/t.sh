echo "SELECT sum (  0"
while read f; do
	echo "+ (case when $f is null then 0 else len($f) end)"
done

echo " ) as Length"
echo "FROM CurrentData.dbo.AWS_ResearchData_STI";
