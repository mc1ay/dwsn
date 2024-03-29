/**
 * @file    main.c
 * @brief   Dynamic Wireless Networking Simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <ini.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "node.h"
#include "mcu_emulation.h"
#include "file_output.h"
#include "settings.h"
#include "state.h"
#include "ground.h"

struct Settings settings;
struct State state;

int main(int argc, char **argv) {
    // Initialization and defaults
    int ret = 0;
    set_program_defaults();

    // If .ini file exists parse it
    if(access("dwsn.ini", F_OK ) == 0 ) {
        if (settings.verbose) {
            printf("Processing settings from dwsn.ini: ");
        } 
        if (ini_parse("dwsn.ini", inih_handler, &settings) < 0) {
            printf("Error reading 'dwsn.ini'\n");
            return 1;
        }
        else {
            if (settings.verbose) {
                printf("OK\n");
            } 
        }
    }

    // get command line switches
    get_switches(argc, argv);

    // state initialization
    initialize_state();

    // Print message about debug level
    if (settings.debug) {
        printf("Debug level: %d\n", settings.debug);
    }

    // Seed random number generator if seed isn't specified
    if (settings.random_seed < 0) {
        settings.random_seed = time(NULL);
    }
    srand(settings.random_seed); 
    if (settings.verbose) {
        printf("Seeded random number generator with: %d\n", settings.random_seed);
    }

    // Output parameters
    if (settings.verbose) {
        printf("Number of nodes: %d\n", settings.node_count);
        printf("Gravity: %f m/(s^2)\n", settings.gravity);
        printf("Time resolution: %f secs/tick\n", settings.time_resolution); 
        printf("Starting height: %f meters\n", settings.start_z);
        printf("Terminal velocity: %f meters/second\n", settings.terminal_velocity);
        printf("Spread factor: %f\n", settings.spread_factor);
        printf("Default power output: %f\n", settings.default_power_output);
        printf("Broadcast percentage: %d\n", settings.broadcast_percentage);
    }
    
    if (settings.output) {
        // Make log directory if output option is turned on
        create_log_dir();
        // create transmit_history file and header
        create_transmit_history_file();
        // create log of received messages at ground
        create_ground_received_file();
    }

    // Get ground station ready
    if (settings.verbose) {
        printf("Initializing ground station: ");
    }
    struct Ground_Station ground;
    ret = initialize_ground(&ground);
    if (ret == 0) {
        if (settings.verbose) {
            printf("OK\n");
        }
    }

    // Get nodes ready
    if (settings.verbose) {
        printf("Initializing nodes: ");
    }
    struct Node nodes[settings.node_count];
    ret = initialize_nodes(nodes);
    if (ret == 0) {
        if (settings.verbose) {
            printf("OK\n");
        }
        state.moving_nodes = settings.node_count;
    }
    
    // Run until all nodes reach z = 0;
    if (settings.verbose) {
        printf("Running simulation\n");
    }

    while (state.moving_nodes != 0) {
        clock_tick(nodes, &ground);
        state.moving_nodes = 0; 
        for (int i = 0; i < settings.node_count; i++) {
            if (nodes[i].z_pos > 0) {
                state.moving_nodes++;
            }
        }
    }

    // Calculate simulation time
    double runTime = (double)(clock() - state.start_time) / CLOCKS_PER_SEC;

    // Print summary information
    if (settings.verbose) {
        printf("Simulation complete\n");
        printf("Cycles processed: %lu\n", state.current_cycle);
        printf("Simulation time: %f seconds\n", state.current_time);        
    }

    if (settings.debug) {
        for (int i = 0; i < settings.node_count; i++) {
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
    if (settings.verbose) {
        printf("Final clock time: %f seconds\n", runTime);
    }
    
    if (settings.verbose) {
        printf("Total collisions detected: %d\n", state.collisions);
    }

    if (settings.verbose) {
        printf("Total messages sent: %lu\n", state.sent_messages);
    }

    if (settings.verbose) {
        printf("Ground station received %d messages\n", ground.messages_received);
    }

    if (settings.verbose) {
        printf("Ground station detected %d collisions\n", ground.collisions_detected);
    }

    if (settings.verbose) {
        printf("Message succeess rate: %f\n", (float)ground.messages_received / state.sent_messages);
    }
    return 0;
}