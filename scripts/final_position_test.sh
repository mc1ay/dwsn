#!/bin/bash

# Description: Test for final positions of nodes after falling to the ground
# Author: Mitchell Clay
# Date: 12/20/2020

# Variables
sensor_count=50
z_height=30000
terminal_velocity=8.0
spread_factor=20

dir=output/final-position-tests/$(date +"%Y-%m-%d-%H-%M-%S")
mkdir -p $dir
./dwsn -d1 -c$sensor_count -z$z_height -t$terminal_velocity -s$spread_factor | grep 'final\ position' | \
awk '{print $11,$12}' >> $dir/output.txt
