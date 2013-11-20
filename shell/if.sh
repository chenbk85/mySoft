function test()
{
	return 1;
}

i=0
test
#echo "$?"
if [ $i -eq 1 ]; then
	echo "if-r(succ)=$?"
	echo "OK";
else
	echo "if-r(fail)=$?"
fi

r=$?
echo "r=$r"
