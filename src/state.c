/**
 * @file    state.c
 * @brief   State information functions
 *
 * @author  Mitchell Clay
 * @date    2/6/2021
**/

#include "node.h"
#include "file_output.h"
#include "mcu_emulation.h"

extern struct Settings settings;
extern struct State state;

int clock_tick(struct Node* nodes, double* current_time) {
    *current_time += settings.time_resolution; 
    
    if (settings.debug > 1) {
        printf("Clock tick: %f\n", *current_time);
    }

    update_acceleration(nodes);
    update_velocity(nodes);
    update_position(nodes);
    update_mcu(nodes, current_time);
    if (settings.output) {
        check_write_interval(nodes, current_time);
    }

    return 0;
}