/**
 * @file    settings.c
 * @brief   Program settings functions/structures 
 *
 * @author  Mitchell Clay
 * @date    2/2/2021
**/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "settings.h"

extern struct Settings settings;

void set_program_defaults() {
    settings.node_count = 5;
    settings.gravity = 9.80665;
    settings.start_x = 0;
    settings.start_y = 0;
    settings.start_z = 30000;
    settings.time_resolution = 0.001;
    settings.terminal_velocity = 8.0;
    settings.spread_factor = 20;
    settings.default_power_output = 400;
    settings.write_interval = 1.0;
    settings.group_max = 5;
    settings.random_seed = -1;
    settings.debug = 0;
    settings.verbose = 1;
    settings.output = 0;
    settings.channels = 16;
    settings.broadcast_percentage = 20;
    settings.output_dir = malloc(sizeof(char) * 50);
    settings.use_pthreads = 0;
    settings.group_cycle_interval = 100000;
}

void get_switches(int argc, char **argv) {
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:e:p:o:m:b:i:")) != -1)
    switch (c) {
        case 'd':
            settings.debug = atoi(optarg);
            break;
        case 'v':
            settings.verbose = atoi(optarg);
            break;
        case 'c':
            settings.node_count = atoi(optarg);
            break;
        case 'g':
            settings.gravity = atof(optarg);
            break;
        case 'r':
            settings.time_resolution = atof(optarg);
            break;
        case 'z':
            settings.start_z = atof(optarg);
            break;
        case 't':
            settings.use_pthreads = atof(optarg);
            break;
        case 's': 
            settings.spread_factor = atof(optarg);
            break;
        case 'e':
            settings.random_seed = atoi(optarg);
            break;
        case 'p':
            settings.default_power_output = atof(optarg);
            break;
        case 'o':
            settings.output = atoi(optarg);
            break;
        case 'm':
            settings.group_max = atoi(optarg);
            break;
        case 'b':
            settings.broadcast_percentage = atoi(optarg);
            break;  
        case 'i':
            settings.group_cycle_interval = atoi(optarg);
            break;     
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else {
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
                perror("unknown_option");
                exit (EXIT_FAILURE);
            }
        default:
            abort ();
    }
}