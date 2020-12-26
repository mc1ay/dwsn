/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

int update_mcu(struct Node* nodes, int node_count, double time_resolution, int debug) {
    for (int i = 0; i < node_count; i++) {
        mcu_run_function(nodes, node_count, i, time_resolution, debug);
    }
    return 0;
}

int mcu_run_function(struct Node* nodes, int node_count, int id, double time_resolution, int debug) {
    int busy_time = 0;

    switch (nodes[id].current_function) {
        case 0:                         // initial starting point for all nodes
            if (id % 2 == 0) {          // send half to function 1 and half to function 2
                push(0, &nodes[id].function_stack);
                nodes[id].current_function = 1;
            }
            else {
                push(0, &nodes[id].function_stack);
                nodes[id].current_function = 2;
            }
            break;
        case 1:
            busy_time = .05;            // assuming 50 ms listen time per channel, update later
            mcu_function_scan_lfg(nodes, node_count, id, time_resolution, busy_time, debug);
            break;
        case 2:
            busy_time = 1.0;            // broadcast LFG for 1.0 seconds
            mcu_function_broadcast_lfg(nodes, id, time_resolution, busy_time, debug);
            break;
        case 3:
            busy_time = .05;            // assuming 50ms listen time per channel
            mcu_function_find_clear_channel(nodes, node_count, id, time_resolution, busy_time, debug);
            break;
        default:
            abort ();
    }
    return 0;
}

int mcu_function_scan_lfg(struct Node* nodes, int node_count, int id, double time_resolution, double busy_time, int debug) {
    if (nodes[id].busy_remaining == 0) {     // if busy_time is zero node just entered this function
        nodes[id].busy_remaining = busy_time;
    }
    else if (nodes[id].busy_remaining <= busy_time && nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;    
        }
        else {                               // see if a node is transmitting on the same channel
            for (int i = 0; i < node_count; i++) {
                if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
                    update_signal(nodes, id, i, debug);
                } 
            }
            if (nodes[id].active_channel < 64) {       // go to next channel if less than max_channels (using 64 for now)
                nodes[id].active_channel++;
            }
            else {                           // for now, if done scanning, reset node
                nodes[id].busy_remaining = 0;    
                nodes[id].current_function = 0;
                nodes[id].active_channel = 0;
            }
        }
    }
    return 0;
}

int mcu_function_broadcast_lfg(struct Node* nodes, int id, double time_resolution, double busy_time, int debug) {
    if (nodes[id].busy_remaining == 0) {     // if busy_time is zero node just entered this function
        nodes[id].busy_remaining = busy_time;
    }
    else if (nodes[id].busy_remaining < busy_time && nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;    
        }
        else {                               // end of cycle
            if (nodes[id].transmit_active) { // continue transmitting
                // do nothing for now except transmit
            }
            else {                           // look for clear channel
                push(1, &nodes[id].function_stack);
                nodes[id].current_function = 3;
            }
        }
    }
    return 0;
}

int mcu_function_find_clear_channel(struct Node* nodes, int node_count, int id, double time_resolution, double busy_time, int debug) {
    if (nodes[id].busy_remaining == 0) {     // if busy_time is zero node just entered this function
        nodes[id].busy_remaining = busy_time;
    }
    else if (nodes[id].busy_remaining < busy_time && nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;    
        }
        else { 
            for (int i=0; i < node_count; i++) {
                if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active) {
                    if (nodes[id].active_channel < 64) {
                        nodes[id].active_channel++;
                    }
                    else {
                        nodes[id].active_channel = 0;
                    }
                }
                else {
                    nodes[id].current_function = nodes[id].function_stack->data; // found clear channel
                    pop(&nodes[id].function_stack);
                    nodes[id].transmit_active = 1;
                }
            }
        }
    }
    return 0;
}