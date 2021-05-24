#/bin/bash

# Description: Test for message success rate for sensor data received
#              by ground station
# Author: Mitchell Clay
# Date: 5/24/2021

# Variables
debug=1
max_broadcast_percent=100

dir=output/received_test/$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p $dir

total=0.0
nodes=100
repeat_times=10
repeat=0
broadcast_percent=1

while [ $broadcast_percent -le $max_broadcast_percent ];
do
    while [ $repeat -lt $repeat_times ];
    do
        cmd="./dwsn -c$nodes -v1 -b$broadcast_percent -m$nodes"
        if [ $debug -gt 0 ]
            then echo "Running \"$cmd\"" 
        fi
        success=$(eval $cmd | grep 'Message success rate' | cut -d" " -f4)
        total=$(echo "$total + $success" | bc)
        ((repeat=repeat+1))
        echo "Run $repeat, $broadcast_percent percent success rate: $success"
    done
    average=$(echo "$total / $repeat_times" | bc -l)
    echo "$broadcast_percent $average" >> $dir/output.txt
    repeat=0
    total=0.0
    ((broadcast_percent=broadcast_percent+1))
done

gnuplot -p -e "plot '$dir/output.txt' with linespoints"
