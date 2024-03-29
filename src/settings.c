/**
 * @file    settings.c
 * @brief   Program settings functions/structures 
 *
 * @author  Mitchell Clay
 * @date    2/2/2021
**/

#include <ctype.h>
#include <ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    settings.default_power_output = 20;
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
    settings.use_timeslots = 1;
    settings.group_cycle_interval = 20000;
    settings.sensor_count = 0;
}

int inih_handler(void* user, const char* section, const char* name,
                   const char* value)
{
    struct Settings* pconfig = (struct Settings*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("program", "node_count")) {
        pconfig->node_count = atoi(value);
    } else if (MATCH("program", "gravity")) {
        pconfig->gravity = atof(value);
    } else if (MATCH("program", "time_resolution")) {
        pconfig->time_resolution = atof(value);
    } else if (MATCH("program", "broadcast_percentage")) {
        pconfig->broadcast_percentage = atoi(value);        
    } else if (MATCH("program", "use_pthreads")) {
        pconfig->use_pthreads = atoi(value);        
    } else if (MATCH("program", "use_timeslots")) {
        pconfig->use_timeslots = atoi(value);        
    } else if (MATCH("program", "seed")) {
        pconfig->random_seed = atoi(value);        
    } else if (MATCH("program", "group_cycle_interval")) {
        pconfig->group_cycle_interval = atoi(value);        
    } else if (MATCH("file_output", "output")) {
        pconfig->output = atoi(value);        
    } else if (MATCH("file_output", "write_interval")) {
        pconfig->write_interval = atof(value);        
    } else if (MATCH("terminal_output", "verbose")) {
        pconfig->verbose = atoi(value);        
    } else if (MATCH("terminal_output", "debug")) {
        pconfig->debug = atoi(value);        
    } else if (MATCH("nodes", "start_x")) {
        pconfig->start_x = atof(value);        
    } else if (MATCH("nodes", "start_y")) {
        pconfig->start_y = atof(value);        
    } else if (MATCH("nodes", "start_z")) {
        pconfig->start_z = atof(value);        
    } else if (MATCH("nodes", "terminal_velocity")) {
        pconfig->terminal_velocity = atof(value);        
    } else if (MATCH("nodes", "spread_factor")) {
        pconfig->spread_factor = atof(value);        
    } else if (MATCH("nodes", "power_output")) {
        pconfig->default_power_output = atof(value);        
    } else if (MATCH("nodes", "group_max")) {
        pconfig->group_max = atoi(value);        
    } else if (MATCH("nodes", "channels")) {
        pconfig->channels = atoi(value);   
    } else if (MATCH("nodes", "sensors")) {
        pconfig->sensor_count = atoi(value);  
        pconfig->sensor_types = malloc(sizeof(int));
    } else if (MATCH("sensor1", "type")) {
        pconfig->sensor_types[0] = atoi(value); 
    } else if (MATCH("sensor2", "type")) {
        pconfig->sensor_types[1] = atoi(value);   
    } else if (MATCH("sensor3", "type")) {
        pconfig->sensor_types[2] = atoi(value);
    } else if (MATCH("sensor4", "type")) {
        pconfig->sensor_types[3] = atoi(value);        
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

void get_switches(int argc, char **argv) {
    int c;
    while ((c = getopt(argc, argv, "d:v:c:g:r:z:t:s:e:p:o:m:b:i:l:")) != -1)
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
        case 'l':
            settings.use_timeslots = atoi(optarg);
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