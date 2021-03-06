/**
 * @file    ground.h
 * @brief   Ground station specific functions/structs
 *
 * @author  Mitchell Clay
 * @date    3/6/2021
**/

#ifndef ground_H
#define ground_H

struct Ground_Station {
    int messages_received;
    int collisions_detected;
};

int initialize_ground(struct Ground_Station* ground);

#endif