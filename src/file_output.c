/**
 * @file    file_output.c
 * @brief   File output functions 
 *
 * @author  Mitchell Clay
 * @date    1/3/2021
**/

#include "file_output.h"

int check_write_interval(struct Node* nodes,
                         int node_count,
                         int channels,
                         double *current_time, 
                         double time_resolution, 
                         double write_interval,
                         char* output_dir,
                         int debug) {

    char file_path[100];
    char channel_active[channels * 2 + 1];
    
    if (fmod(*current_time, write_interval) < time_resolution) {
        for (int i = 0; i < node_count; i++) {
            // output node specific info into one file per node
            sprintf(file_path, "%s/node-%d%s", output_dir, i, ".txt");
            FILE *node_data_file;
            node_data_file  = fopen (file_path, "a");
            write_node_data(nodes, node_count, i, *current_time, node_data_file);
            fclose(node_data_file);
        }
        // Open transmit_history file for writing
        sprintf(file_path, "%s/transmit_history.txt", output_dir);
        FILE *transmit_history_file;
        transmit_history_file = fopen (file_path, "a");
        // Build line of output for this timeslice
        char buffer[sizeof(channels) * 2 + 20];
        for (int i = 0; i < channels; i++) {
            for (int j = 0; j < node_count; j++) {
                if (nodes[j].active_channel == i && nodes[j].transmit_active == 1) {
                    channel_active[i * 2] = 88;
                }
                else {
                    channel_active[i * 2] = 46;
                }
                if (i < channels - 1) {
                    // Remove trailing tab or else we get garbage output
                    channel_active[i * 2 + 1] = 9;
                }
            }
        }
        // Write output line to file and close
        sprintf(buffer, "%f\t%s\n", *current_time, channel_active);
        fputs(buffer, transmit_history_file);
        fclose(transmit_history_file);
    }
    return 0;
}