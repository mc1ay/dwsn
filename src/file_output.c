/**
 * @file    file_output.c
 * @brief   File output functions 
 *
 * @author  Mitchell Clay
 * @date    1/3/2021
**/

#include "file_output.h"

extern struct Settings settings;
extern struct State state;

int create_log_dir() {
    struct tm *timenow;
    time_t now = time(NULL);
    timenow = gmtime(&now);
    strftime(settings.output_dir, sizeof(char) * 50, "output/run/%Y-%m-%d-%H-%M-%S", timenow);
    if (settings.verbose) {
        printf("Creating output directory \"%s\": ", settings.output_dir);
    }

    // Make path for timestamped directory if it doesn't already exist
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        mkdir("output", 0777);
    }
    if (stat("output/run", &st) == -1) {
        mkdir("output/run", 0777);
    }

    // Make directory just for this run
    int ret = mkdir(settings.output_dir,0777); 

    // check if directory is created or not 
    if (!ret) {
        if (settings.verbose) {
            printf("OK\n"); 
        }
    }
    else { 
        if (settings.verbose) {
            printf("Unable to create directory, exiting\n"); 
        }
        exit(1); 
    }
    return 0; 
}

int create_transmit_history_file() {
    char file_path[100];
    sprintf(file_path, "%s/transmit_history.txt", settings.output_dir);
    FILE *transmit_history_file;
    transmit_history_file = fopen (file_path, "a");
    // Build line of output for this clock tick
    char buffer[10];
    sprintf(buffer, "Time\t\t");
    fputs(buffer, transmit_history_file);
    for (int i = 0; i < settings.channels; i++) {
        if (i < settings.channels -1) {
            sprintf(buffer, "%d\t", i);
            fputs(buffer, transmit_history_file);

        }
        else {
            sprintf(buffer, "%d", i);
            fputs(buffer, transmit_history_file);
        }
    }
    sprintf(buffer, "\n");
    fputs(buffer, transmit_history_file);
    fclose(transmit_history_file);

    return 0;
}

int check_write_interval(struct Node* nodes, double *current_time) {

    char file_path[100];
    char channel_active[settings.channels * 2 + 1];

    if (settings.debug > 1) {
        printf("debug level: %d\n", settings.debug);
        printf("Checking write interval: ");
    }
    
    if (fmod(*current_time, settings.write_interval) < settings.time_resolution) {
        if (settings.debug > 1) {
            printf ("Match, writing output\n");
        }
        for (int i = 0; i < settings.node_count; i++) {
            // output node specific info into one file per node
            sprintf(file_path, "%s/node-%d%s", settings.output_dir, i, ".txt");
            FILE *node_data_file;
            
            if (settings.debug> 1) {
                printf("Opening %s for append\n", file_path);
            }
            node_data_file  = fopen (file_path, "a");

            if (settings.debug> 1) {
                printf("Writing data to file\n");
            }
            write_node_data(nodes, i, *current_time, node_data_file);

            if (settings.debug> 1) {
                printf("Closing file\n");
            }
            fclose(node_data_file);
        }
        // Open transmit_history file for writing
        sprintf(file_path, "%s/transmit_history.txt", settings.output_dir);
        FILE *transmit_history_file;
        if (settings.debug> 1) {
            printf("Opening %s for append\n", file_path);
        }
        transmit_history_file = fopen (file_path, "a");
        // Build line of output for this timeslice
        char buffer[sizeof(settings.channels) * 2 + 100];
        if (settings.debug> 1) {
            printf("Allocated buffer\n");
        }
        int transmit_count = 0;
        for (int i = 0; i < settings.channels; i++) {
            if (settings.debug> 1) {
                printf("Checking channel %d\n", i);
            }
            // Do this for each channel
            for (int j = 0; j < settings.node_count; j++) {
                // Check each node to see if it is transmitting on channel
                if (nodes[j].active_channel == i && nodes[j].transmit_active == 1) {
                    transmit_count++;
                }
            }
            if (transmit_count > 0) {
                // If any nodes were transmitting on this channel output an "X"
                // into history file and reset counter
                channel_active[i * 2] = 88;
                transmit_count = 0;
            }
            else {
                // If no signals detected, output a "."
                channel_active[i * 2] = 46;
            } 
            if (i < settings.channels - 1) {
                // Remove trailing tab or else we get garbage output
                channel_active[i * 2 + 1] = 9;
            }
        }
        // Write output line to file and close
        if (settings.debug> 1) {
            printf("Putting line into buffer\n");
        }
        sprintf(buffer, "%f\t%s\n", *current_time, channel_active);

        if (settings.debug> 1) {
            printf("Writing data to file\n");
        }
        fputs(buffer, transmit_history_file);

        if (settings.debug> 1) {
            printf("Closing file\n");
        }
        fclose(transmit_history_file);
    }
    else {
        if (settings.debug> 1) {
            printf("Not at interval\n");
        }
    }
    return 0;
}