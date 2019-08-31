#!/bin/bash

awk -F ' ' '
BEGIN {anom=0}
{
a[$2]+=$3;
success+=$4;
fail+=$5;
if ($5 > 0) anom+=1
}
END {
for (i in a)
	print i "\nTotal operations: ",
	a[i]"\nSuccessful operations: " success "\nFailed operations: " fail "\nAnomalous clients: "anom
}' testout.log
