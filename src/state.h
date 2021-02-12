/**
 * @file    state.h
 * @brief   State information struct/functions
 *
 * @author  Mitchell Clay
 * @date    2/6/2021
**/

#include <time.h>
#include "node.h"

#ifndef state_H
#define state_H

// Struct for storing program state 
struct State {
    clock_t start_time;
    int moving_nodes; 
    double current_time;
    int collisions;
};

int initialize_state();
int clock_tick(struct Node* nodes);

#endif