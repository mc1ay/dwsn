/**
 * @file    ground.c
 * @brief   Ground station specific functions
 *
 * @author  Mitchell Clay
 * @date    3/6/2021
**/

#include <string.h>
#include "file_output.h"
#include "ground.h"
#include "settings.h"
#include "state.h"

extern struct Settings settings;
extern struct State state;

int initialize_ground(struct Ground_Station* ground) {
    ground->messages_received = 0;
    ground->collisions_detected = 0;
    ground->x_pos = 0.0;
    ground->y_pos = 0.0;
    ground->z_pos = 0.0;

    return 0;
}

int update_ground(struct Node* nodes, struct Ground_Station* ground) {
    int signals_detected = 0;
    int transmitting_node = -1;

    // Scan each channel
    for (int i = 0; i < settings.channels; i++) {
        // Check each node
        // Later add signal strength update/checking
        for (int j = 0; j < settings.node_count; j++) {
            if (nodes[j].transmit_active == 1 && nodes[j].active_channel == i) {
                signals_detected++;
                transmitting_node = j;
            }
        }
        // Check for collision
        if (signals_detected > 1) {
            ground->collisions_detected++;
        }
        else if (signals_detected == 1) {
            // Check for DATA message
            char* token;
            char incoming_buffer[256];
            char message[256];

            // Zero out message string to eliminate proceeded garbage data
            bzero(message, 256);

            strncpy(incoming_buffer, nodes[transmitting_node].send_packet, 256);
            token = strtok(incoming_buffer, " ");
            char sender_id[6];
            
            // Extract dest and src node IDs from message
            if (strcmp(token, "GROUND") == 0) {
                token = strtok(NULL, " ");
                strncpy(sender_id, token, 6);

                // Check if third token is "DATA"
                token = strtok(NULL, " ");
                if (strcmp(token, "RELAY") == 0) {
                    // Process remaining tokens
                    token = strtok(NULL, " ");
                    do {
                        strncat(message, token, strlen(token));
                        strncat(message, " ", 2);
                        token = strtok(NULL, " ");
                    } while (token != 0);

                    // Update message counter and write to file if output flag set
                    ground->messages_received++;
                    if (settings.output) {
                        // write to log file
                        log_ground_received_message(message, strlen(message));
                    }
                }
            }
        }
    }
    return 0;
}
