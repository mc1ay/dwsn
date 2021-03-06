/**
 * @file    ground.c
 * @brief   Ground station specific functions
 *
 * @author  Mitchell Clay
 * @date    3/6/2021
**/

#include "ground.h"
#include "settings.h"
#include "state.h"

extern struct Settings settings;
extern struct State state;

int initialize_ground(struct Ground_Station* ground) {
    ground->messages_received = 0;
    ground->collisions_detected = 0;

    return 0;
}

int update_ground(struct Node* nodes, struct Ground_Station* ground) {
    int signals_detected = 0;
    int transmitting_node = -1;

    // Scan each channel
    for (int i = 0; i < settings.channels; i++) {
        // Check each node
        // Later add signal strength update/checking
        for (int j = 0; i < settings.node_count; i++) {
            if (nodes[j].transmit_active == 1 && nodes[j].active_channel == i) {
                signals_detected++;
                transmitting_node = i;
            }
        }
        // Check for collision
        if (signals_detected > 1) {
            ground->collisions_detected++;
        }
        else {
            // process messages
        }
    }
    return 0;
}
