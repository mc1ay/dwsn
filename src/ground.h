/**
 * @file    ground.h
 * @brief   Ground station specific functions/structs
 *
 * @author  Mitchell Clay
 * @date    3/6/2021
**/

#include "node.h"
#include "settings.h"

#ifndef ground_H
#define ground_H

struct Ground_Station {
    int messages_received;
    int collisions_detected;
    double x_pos;
    double y_pos;
    double z_pos;
    int* new_message_available;
};

int initialize_ground(struct Ground_Station* ground);
int update_ground(struct Node* nodes, struct Ground_Station* ground);

#endif