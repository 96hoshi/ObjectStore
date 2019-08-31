#!/bin/bash

awk -F ' ' '
BEGIN {anom=0}
{
a[$1]+=$2;
success+=$3;
fail+=$4;
if ($4 > 0) anom+=1
}
END {
for (i in a)
	print i "\nTotal operations: ",
	a[i]"\nSuccessful operations: " success "\nFailed operations: " fail "\nAnomalous clients: "anom
}' testout.log
