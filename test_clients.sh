#!/bin/bash

./objStore 2> /dev/null &
serverpid=$!
sleep 1

for i in `seq 1 50`; do
	./client client${i} 1 1>> testout.log 2> /dev/null &
	pids[${i}]=$!
done

# wait for all pids
for pid in ${pids[*]}; do
	wait $pid
done

for i in `seq 1 30`; do
	./client client${i} 2 1>> testout.log 2> /dev/null &
	pids[${i}]=$!
done

for i in `seq 31 50`; do
	./client client${i} 3 1>> testout.log 2> /dev/null &
	pids[${i}]=$!
done

# wait for all pids
for pid in ${pids[*]}; do
	wait $pid
done

kill -SIGUSR1 $serverpid
kill -SIGTERM $serverpid
