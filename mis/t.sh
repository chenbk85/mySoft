i=$#;

echo "ARGV=$0";
echo $i

exec 5>&1-
ls
while read -u 5 line; do echo "#5: $line"; done
exec 5<&- # close it
