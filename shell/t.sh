#!/bin/bash

#if [[ $# < 10 ]]
if [ "$#" < "1" ]
#if (($# < 1))
  then
    echo "Usage:exchangeId.sh minutes";
    exit 
fi

echo "another line";

b="a,b,c d"
aa=
eval "aa=($b)"
echo ${aa[0]};

t="k";
if [ $t = "t" ]; then
	echo "match t*"
else
	echo "does not match t*"
fi
