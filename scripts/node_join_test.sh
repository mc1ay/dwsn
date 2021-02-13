#!/bin/bash

# Description: Test for successful node joining
# Author: Mitchell Clay
# Date: 2/12/2021

# Variables
node_count_max=30
broadcast_nodes=1
debug=0

dir=output/join_test/$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p $dir

total=0.0
nodes=$(echo $broadcast_nodes + 1 | bc -l)
repeat_times=10
repeat=0

while [ $nodes -lt $(expr $node_count_max + 1) ];
do
    while [ $repeat -lt $repeat_times ];
    do
        cmd="./dwsn -d1 -c$(expr $nodes) -d1 -b$broadcast_nodes -m$nodes"
        if [ $debug -gt 0 ]
            then echo "Running \"$cmd\" with $(bc -l <<< "($nodes - $broadcast_nodes)") node(s)"
        fi
        joined=$(bc -l <<< "scale=2;$(eval $cmd| grep 'added' | wc -l) / ($nodes - $broadcast_nodes)")
        total=$(echo "$total + $joined" | bc)
        ((repeat=repeat+1))
        echo "Run $repeat, $nodes nodes: $(echo "$joined * 100" | bc)% of $(expr $nodes - $broadcast_nodes) successfully joined"
    done
    average=$(echo "$total / $repeat_times" | bc -l)
    echo "$nodes $average" >> $dir/output.txt
    repeat=0
    total=0.0
    ((nodes=nodes+1))
done

gnuplot -p -e "plot '$dir/output.txt' with linespoints"