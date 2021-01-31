/**
 * @file    settings.h
 * @brief   Program settings functions/structures 
 *
 * @author  Mitchell Clay
 * @date    1/31/2021
**/

#include <time.h>

#ifndef settings_H
#define settings_H

// Struct for storing program settings
struct Settings {
    int node_count;
    double gravity;
    double start_x;
    double start_y;
    double start_z;
    double time_resolution;
    double terminal_velocity;
    double spread_factor;
    double default_power_output;
    double write_interval;
    int debug;
    int verbose;
    int output;
    int channels;
    int initial_broadcast_nodes;
    char* output_dir;
};

// Struct for storing program state 
struct State {
    clock_t start_time;
    int moving_nodes; 
    int ret;
    double current_time;
};

#endif