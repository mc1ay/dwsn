#!/bin/bash

# Description: Test for starting z height vs spread of nodes at ground position
# Author: Mitchell Clay
# Date: 12/20/2020

# Variables
sensor_count=50
starting_z_height=30000
terminal_velocity=8.0
spread_factor=20
interval=100
z_height=$starting_z_height

dir=output/startz_vs_spread/$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p $dir

total=0.0
count=0
while [ $z_height -gt 0 ];
do
    ./dwsn -d1 -c$sensor_count -z$z_height -t$terminal_velocity -s$spread_factor | grep 'final\ position' | \
    awk '{print $11,$12}' |
    while read -r line; 
        do 
        ((count=count+1))
        x_coordinate=$(echo $line | cut -f1 -d' ')
        y_coordinate=$(echo $line | cut -f2 -d' ')
        distance=$(echo "sqrt($x_coordinate^2 + $y_coordinate^2)" | bc)
        export total=$(echo "$total + $distance" | bc)
        if [ $count == $sensor_count ]
        then
            average=$(echo "$total / $count" | bc)
            echo "$z_height $average" >> $dir/output.txt
        fi
    done 
    ((z_height=z_height-interval))
done
