/**
 * @file    main.c
 * @brief   Dynamic Wireless Networking Simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "node.h"
#include "mcu_emulation.h"
#include "file_output.h"

int clock_tick(struct Node* nodes, 
               int node_count, 
               double* current_time, 
               double time_resolution, 
               double gravity,
               double spread_factor,
               int debug,
               int output,
               char* output_dir,
               int write_interval,
               int group_max,
               int channels) {
    *current_time += time_resolution; 
    
    if (debug > 1) {
        printf("Clock tick: %f\n", *current_time);
    }

    update_acceleration(nodes, node_count, time_resolution, spread_factor, debug);
    update_velocity(nodes, node_count, time_resolution, debug);
    update_position(nodes, node_count, time_resolution, debug);
    update_mcu(nodes, node_count, time_resolution, group_max, channels, debug);
    if (output) {
        check_write_interval(nodes, node_count, channels, current_time, 
                             time_resolution, write_interval, output_dir, debug);
    }

    return 0;
}

int main(int argc, char **argv) {
    // Initialization and defaults
    clock_t t1, t2;
    int node_count = 20;
    int moving_nodes = 0; 
    int ret = 0;
    double gravity = 9.80665;
    double start_x = 0;
    double start_y = 0;
    double start_z = 30000;
    double current_time = 0;
    double time_resolution = 0.001;
    double terminal_velocity = 8.0;
    double spread_factor = 20;
    double default_power_output = 400;
    double write_interval = 1.0;
    int group_max = 5;
    int random_seed = -1;
    int debug = 0;
    int verbose = 1;
    int output = 0;
    int channels = 16;
    char* output_dir = malloc(sizeof(char) * 50);
    
    // get command line switches
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:e:p:o:m:")) != -1)
    switch (c) {
        case 'd':
            debug = atoi(optarg);
            break;
        case 'v':
            verbose = atoi(optarg);
            break;
        case 'c':
            node_count = atoi(optarg);
            break;
        case 'g':
            gravity = atof(optarg);
            break;
        case 'r':
            time_resolution = atof(optarg);
            break;
        case 'z':
            start_z = atof(optarg);
            break;
        case 't':
            terminal_velocity = atof(optarg);
            break;
        case 's': 
            spread_factor = atof(optarg);
            break;
        case 'e':
            random_seed = atoi(optarg);
            break;
        case 'p':
            default_power_output = atof(optarg);
            break;
        case 'o':
            output = atoi(optarg);
            break;
        case 'm':
            group_max = atoi(optarg);
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
                return 1;
            }
        default:
            abort ();
    }
    
    // Print message about debug level
    if (debug) {
        printf("Debug level: %d", debug);
    }

    // Seed random number generator if seed isn't specified
    if (random_seed < 0) {
        srand(time(NULL)); 
        if (verbose) {
            printf("Seeded random number generator\n");
        }
    }

    // Output parameters
    if (verbose) {
        printf("Number of nodes: %d\n", node_count);
        printf("Gravity: %f m/(s^2)\n", gravity);
        printf("Time resolution: %f secs/tick\n", time_resolution); 
        printf("Starting height: %f meters\n", start_z);
        printf("Terminal velocity: %f meters/second\n", terminal_velocity);
        printf("Spread factor: %f\n", spread_factor);
        printf("Default power output: %f\n", default_power_output);
    }
    
    if (output) {
        // Make log directory if output option is turned on
        create_log_dir(output_dir, verbose);
        // create transmit_history file and header
        create_transmit_history_file(output_dir, channels);
    }

    // Get nodes ready
    if (verbose) {
        printf("Initializing nodes\n");
    }
    struct Node nodes[node_count];
    ret = initialize_nodes(nodes, node_count, terminal_velocity, 
                           start_x, start_y, start_z, gravity, 
                           default_power_output, output, output_dir, 
                           group_max, channels, debug);
    if (ret == 0) {
        if (verbose) {
            printf("Initialization OK\n");
        }
        moving_nodes = node_count;
    }
    
    // Run until all nodes reach z = 0;
    if (verbose) {
        printf("Running simulation\n");
    }
    t1 = clock();

    while (moving_nodes != 0) {
        clock_tick(nodes, node_count, &current_time, time_resolution, gravity,
                   spread_factor, debug, output, output_dir, write_interval, 
                   group_max, channels);
        moving_nodes = 0; 
        for (int i = 0; i < node_count; i++) {
            if (nodes[i].z_pos > 0) {
                moving_nodes++;
            }
        }
    }

    // Calculate simulation time
    t2 = clock();
    double runTime = (double)(t2 - t1) / CLOCKS_PER_SEC;

    // Print summary information
    if (verbose) {
        printf("Simulation complete\n");
        printf("Simulation time: %f seconds\n", runTime);        
    }

    if (debug) {
        for (int i = 0; i < node_count; i++) {
            printf("Node %d final velocity: %f %f %f m/s, final position: %f %f %f\n", 
                i, 
                nodes[i].x_velocity, 
                nodes[i].y_velocity, 
                nodes[i].z_velocity, 
                nodes[i].x_pos, 
                nodes[i].y_pos, 
                nodes[i].z_pos);
        }
    }
    if (verbose) {
        printf("Final clock time: %f seconds\n", current_time);
    }
    
    return 0;
}