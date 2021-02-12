#!/bin/bash

# Description: Test for successful node joining
# Author: Mitchell Clay
# Date: 2/12/2021

# Variables
node_count_max=12

dir=output/join_test/$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p $dir

total=0.0
nodes=2
repeat_times=100
repeat=0

while [ $nodes -lt $(expr $node_count_max + 1) ];
do
    while [ $repeat -lt $repeat_times ];
    do
        joined=$(bc -l <<< "scale=2;$(./dwsn -d1 -c$nodes -d1 -b1| grep 'added' | wc -l) / ($nodes - 1)")
        total=$(echo "$total + $joined" | bc)
        ((repeat=repeat+1))
        echo "Run $repeat, $nodes nodes: $(echo "$joined * 100" | bc)% of $(expr $nodes - 1) successfully joined"
    done
    average=$(echo "$total / $repeat_times" | bc -l)
    echo "Average: $average"
    echo "$nodes $average" >> $dir/output.txt
    repeat=0
    total=0.0
    ((nodes=nodes+1))
done

gnuplot -p -e "plot '$dir/output.txt' with linespoints"