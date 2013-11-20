c=${1:-20}
s='abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
l=`echo -n $s | wc -c`
i=0; 
while test $i -lt $c; do t=`expr $RANDOM % $l`; echo -n `expr substr $s $t 1`; i=`expr $i + 1`;done;
echo
