/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

int mcu_run_function(struct Node* nodes, int id, double time_resolution, int debug) {
    int busy_time = 0;

    switch (nodes[id].current_function) {
        case 0:                         // initial starting point for all nodes
            if (id % 2 == 0) {          // send half to function 1 and half to function 2
                nodes[id].current_function = 1;
            }
            else {
                nodes[id].current_function = 2;
            }
            break;
        case 1:
            busy_time = .05 * 64;       // assuming 50 ms * 64 channels, update later
            mcu_function_scan_lfg(nodes, id, time_resolution, busy_time, debug);
            break;
        case 2:
            busy_time = 1.0;            // broadcast LFG for 1.0 seconds
            mcu_function_broadcast_lfg(nodes, id, time_resolution, busy_time, debug);
            break;
        default:
            abort ();
    }

    return 0;
}

int mcu_function_scan_lfg(struct Node* nodes, int id, double time_resolution, double busy_time, int debug) {
    if (nodes[id].busy_remaining == 0) {     // if busy_time is zero node just entered this function
        nodes[id].busy_remaining = busy_time;
    }
    else if (nodes[id].busy_remaining < busy_time && nodes[id].busy_remaining > 0) {
        if (nodes[id].busy_remaining - time_resolution > 0) {
            nodes[id].busy_remaining -= time_resolution;    
        }
        else {                          // end of cycle, scan code goes here later
            nodes[id].busy_remaining = 0;    // for now, just re-run this function
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
        else {                          // end of cycle, broadcast code goes here later
            nodes[id].busy_remaining = 0;    // for now, just re-run this function
        }
    }
    return 0;
}