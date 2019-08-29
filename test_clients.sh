#!/bin/bash
./objStore &

for i in `seq 1 50`; do
	./client ${i} 1 &
	pids[${i}]=$!
done

# wait for all pids
for pid in ${pids[*]}; do
	wait $pid
done
