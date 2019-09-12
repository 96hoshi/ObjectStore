#!/bin/bash

awk -F ' ' '
BEGIN {anom=0}
{
total[$1]+=$2;
success[$1]+=$3;
fail[$1]+=$4;
if ($4 > 0) anom+=1
}
END {
for (i in total)
	print i "\nTotal operations: ",
	total[i]"\nSuccessful operations: ",
	success[i] "\nFailed operations: ",
	fail[i] "\nAnomalous clients: " anom
}' testout.log
