#!/bin/bash

t="`date` is executed by `id`"
echo $t;
echo "$t" >>/tmp/test001.log
