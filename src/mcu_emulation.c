/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"
#include "mcu_functions.h"

/**
 * Microcontroller node selection
 * Desc: Cycles through all nodes and calls MCU function handler for each
**/
int update_mcu(struct Node* nodes,
               int node_count,
               double time_resolution,
               int group_max,
               int channels,
               double* current_time,
               int debug) {
    for (int i = 0; i < node_count; i++) {
        // To-do!!! check to make sure nodes aren't on ground
        mcu_run_function(nodes, node_count, i, time_resolution, group_max, 
                         channels, current_time, debug);
    }
    return 0;
}

/**
 * Microcontroller function selection handling
 * Desc: See which function a nodes MCU should be executing and set/check busy times
 *
 * Functions:
 * ----------------
 *  0: main
 *  1: scan_lfg
 *  2: broadcast_lfg
 *  3: find_clear_channel
 *  4: check_channel_busy
 *  5: transmit_message_begin
 *  6: transmit_message_complete
 *  7: receive
 *  8: sleep
 *  9: respond_lfg
 * 10: scan_lfg_responses
**/
int mcu_run_function(struct Node* nodes,
                     int node_count,
                     int id,
                     double time_resolution,
                     int group_max,
                     int channels,
                     double* current_time,
                     int debug) {
    double busy_time = 0.00;

    mcu_update_busy_time(nodes, id, time_resolution, debug);

    if (nodes[id].busy_remaining <= 0) {
        switch (nodes[id].current_function) {
            case 0:                         // initial starting point for all nodes
                mcu_function_main(nodes, node_count, id, group_max, channels, debug);
                break;
            case 1:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;       // assuming 50 ms listen time per channel, update later
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_scan_lfg(nodes, node_count, id, group_max, channels, debug);
                }
                break;
            case 2:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;        
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_broadcast_lfg(nodes, id, group_max, debug);
                }
                break;
            case 3:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;       
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_find_clear_channel(nodes, node_count, id, debug);
                }
                break;
            case 4:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50ms listen time per channel
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_check_channel_busy(nodes, node_count, id, debug);
                }
                break;
            case 5:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;       
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_transmit_message_begin(nodes, node_count, id, debug);
                }
                break;
            case 6:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 2.00;       // broadcast LFG for 2.0 seconds
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_transmit_message_complete(nodes, node_count, id, debug);
                }
                break;
            case 7:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_receive(nodes, node_count, id, debug);
                }
                break;
            case 8:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 1.00;       // sleep for 1.0 second
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_sleep(nodes, node_count, id, debug);
                }
                break;
            case 9:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;       
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_respond_lfg(nodes, node_count, id, debug);
                }
                break;
            case 10:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.00;       
                    nodes[id].busy_remaining = busy_time;
                }
                else {            
                    mcu_function_scan_lfg_responses(nodes, node_count, id, current_time, group_max, debug);
                }
                break;
            default:
                abort ();
        }
    }
    return 0;
}

// update node busy times
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

int mcu_call(struct Node* nodes, int id, int caller, int return_to_label, int function_number) {
    fs_push(caller, return_to_label, &nodes[id].function_stack);
    nodes[id].busy_remaining = -1;
    nodes[id].current_function = function_number;
    return 0;
}

int mcu_return(struct Node* nodes, int id, int function_number, int return_value) {
    nodes[id].current_function = nodes[id].function_stack->caller; 
    rs_push(function_number, nodes[id].function_stack->return_to_label, return_value, &nodes[id].return_stack);
    fs_pop(&nodes[id].function_stack);
    nodes[id].busy_remaining = -1;
    return 0;
}