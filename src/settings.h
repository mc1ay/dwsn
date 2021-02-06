/**
 * @file    settings.h
 * @brief   Program settings functions/structures 
 *
 * @author  Mitchell Clay
 * @date    1/31/2021
**/

#ifndef settings_H
#define settings_H

// Struct for storing program settings
struct Settings {
    int node_count;
    int random_seed;
    double gravity;
    double start_x;
    double start_y;
    double start_z;
    double time_resolution;
    double terminal_velocity;
    double spread_factor;
    double default_power_output;
    double write_interval;
    int group_max;
    int debug;
    int verbose;
    int output;
    int channels;
    int initial_broadcast_nodes;
    char* output_dir;
    int use_pthreads;
};

void set_program_defaults();
void get_switches(int argc, char **argv);

#endif