/**
 * @file    ground.c
 * @brief   Ground station specific functions
 *
 * @author  Mitchell Clay
 * @date    3/6/2021
**/

#include "ground.h"

extern struct Settings Settings;
extern struct State state;

int initialize_ground(struct Ground_Station* ground) {
    ground->messages_received = 0;
    ground->collisions_detected = 0;

    return 0;
}
