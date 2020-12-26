/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

int mcu_run_function(struct Node* nodes, int id, int function_number, int debug) {
    int busy_time = 0;

    switch (function_number) {
        case 1:
            busy_time = .05 * 64;       // assuming 50 ms * 64 channels, update later
            mcu_function_scan_lfg(nodes, id, busy_time, debug);
            break;
        case 2:
            busy_time = 1.0;            // broadcast LFG for 1.0 seconds
            mcu_function_broadcast_lfg(nodes, id, busy_time, debug);
            break;
        default:
            abort ();
    }
    
    return 0;
}

int mcu_function_scan_lfg(struct Node* nodes, int id, double busy_time, int debug) {

    return 0;
}

int mcu_function_broadcast_lfg(struct Node* nodes, int id, double busy_time, int debug) {

    return 0;
}