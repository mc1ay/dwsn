/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

int update_mcu(struct Node* nodes,
               int node_count,
               double time_resolution,
               int group_max,
               int debug) {
    for (int i = 0; i < node_count; i++) {
        mcu_run_function(nodes, node_count, i, time_resolution, group_max, debug);
    }
    return 0;
}

int mcu_run_function(struct Node* nodes,
                     int node_count,
                     int id,
                     double time_resolution,
                     int group_max,
                     int debug) {
    double busy_time = 0.0;

    mcu_update_busy_time(nodes, id, time_resolution, debug);

    if (nodes[id].busy_remaining <= 0) {
        switch (nodes[id].current_function) {
            case 0:                         // initial starting point for all nodes
                mcu_function_main(nodes, node_count, id, time_resolution, group_max, debug);
                break;
            case 1:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50 ms listen time per channel, update later
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_scan_lfg(nodes, node_count, id, time_resolution, busy_time, group_max, debug);
                }
                break;
            case 2:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 2.0;        // broadcast LFG for 2.0 seconds
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_broadcast_lfg(nodes, id, time_resolution, busy_time, group_max, debug);
                }
                break;
            case 3:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50ms listen time per channel
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_find_clear_channel(nodes, node_count, id, time_resolution, busy_time, debug);
                }
                break;
            default:
                abort ();
        }
    }
    return 0;
}

int mcu_function_main(struct Node* nodes,
                     int node_count,
                     int id,
                     double time_resolution,
                     int group_max,
                     int debug) {
    if (id % 2 == 0) {          // send half to function 1 and half to function 2
        mcu_call(nodes, id, 0, 1);
    }
    else {
        mcu_call(nodes, id, 0, 2);
    }
    return 0;
}

int mcu_function_scan_lfg(struct Node* nodes,
                          int node_count,
                          int id,
                          double time_resolution,
                          double busy_time,
                          int group_max,
                          int debug) {
    for (int i = 0; i < node_count; i++) {
        if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
            update_signal(nodes, id, i, debug);
        }
    }
    if (nodes[id].active_channel < 64) {       // go to next channel if less than max_channels (using 64 for now)
        nodes[id].active_channel++;
    }
    else {                           // for now, if done scanning, reset node
        nodes[id].busy_remaining = -1;    
        nodes[id].current_function = 0;
        nodes[id].active_channel = 0;
    }
    return 0;
}

int mcu_function_broadcast_lfg(struct Node* nodes,
                               int id,
                               double time_resolution,
                               double busy_time,
                               int group_max,
                               int debug) {
    if (nodes[id].transmit_active) { 
                nodes[id].busy_remaining = 0;
                nodes[id].transmit_active = 0;
    }
    else {                           // look for clear channel
        mcu_call(nodes, id, 2, 3);
    }
    return 0;
}

int mcu_function_find_clear_channel(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    double time_resolution,
                                    double busy_time,
                                    int debug) {
    for (int i = 0; i < node_count; i++) {
        if (i != id) {                  // don't check own id
            if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active) {
                if (nodes[id].active_channel < 64) {
                    nodes[id].active_channel++;
                    return 0;
                }
                else {
                    nodes[id].active_channel = 0;
                    return 0;
                }
            }
        }
    }
    nodes[id].transmit_active = 1;
    mcu_return(nodes, id, 0);
    return 0;
}

int mcu_update_busy_time(struct Node* nodes,
                         int id,
                         double time_resolution,
                         int debug) {
    if (nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;
        }
        else {
            nodes[id].busy_remaining = 0;
        } 
    }
    return 0;
}

int mcu_call(struct Node* nodes, int id, int caller, int function_number) {
    push(caller, &nodes[id].function_stack);
    nodes[id].busy_remaining = -1;
    nodes[id].current_function = function_number;
    return 0;
}

int mcu_return(struct Node* nodes, int id, int return_value) {
    nodes[id].current_function = nodes[id].function_stack->data; 
    pop(&nodes[id].function_stack);
    nodes[id].return_value = return_value;
    nodes[id].busy_remaining = -1;

    return 0;
}