/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "mcu_emulation.h"

/**
 * Microcontroller node selection
 * Desc: Cycles through all nodes and calls MCU function handler for each
**/
int update_mcu(struct Node* nodes,
               int node_count,
               double time_resolution,
               int group_max,
               int debug) {
    for (int i = 0; i < node_count; i++) {
        // To-do!!! check to make sure nodes aren't on ground
        mcu_run_function(nodes, node_count, i, time_resolution, group_max, debug);
    }
    return 0;
}

/**
 * Microcontroller function selection handling
 * Desc: See which function a nodes MCU should be executing and set/check busy times
 *
 * Functions:
 * ----------------
 * 0: main
 * 1: scan_lfg
 * 2: broadcast_lfg
 * 3: find_clear_channel
 * 4: check_channel_busy
 * 5: transmit_message_begin
 * 6: transmit_message_complete
**/
int mcu_run_function(struct Node* nodes,
                     int node_count,
                     int id,
                     double time_resolution,
                     int group_max,
                     int debug) {
    double busy_time = 0.00;

    mcu_update_busy_time(nodes, id, time_resolution, debug);

    if (nodes[id].busy_remaining <= 0) {
        switch (nodes[id].current_function) {
            case 0:                         // initial starting point for all nodes
                mcu_function_main(nodes, node_count, id, group_max, debug);
                break;
            case 1:
                if (nodes[id].busy_remaining < 0) {
                    busy_time = 0.05;       // assuming 50 ms listen time per channel, update later
                    nodes[id].busy_remaining = busy_time;
                }
                else {
                    mcu_function_scan_lfg(nodes, node_count, id, group_max, debug);
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
            default:
                abort ();
        }
    }
    return 0;
}

/**
 * Function Number:             0
 * Function Name:               main
 * Function Description:        Main Loop of microcontroller emulation routine
 * Function Busy times:         None
 * Function Return Labels:      2
 
 * Return Label 0 returns from: 1 (scan_lfg)
 * Return Lable 0 reason:       Check to see if scan function finds a looking for 
                                group broadcast message
 
 * Return Label 1 returns from: 2 (broadcast_lfg)
 * Return Label 1 reason:       Check to see if LFG was successfully broadcast on
                                an open channel

 * Function Returns:            nothing
**/
int mcu_function_main(struct Node* nodes,
                     int node_count,
                     int id,
                     int group_max,
                     int debug) {
    int own_function_number = 0;

    if (id % 2 == 0) {          // send half to function 1 and half to function 2
        mcu_call(nodes, id, own_function_number, 0, 1);
    }
    else {
        mcu_call(nodes, id, own_function_number, 1, 2);
    }
    return 0;
}

/**
 * Function Number:             1
 * Function Name:               scan_lfg
 * Function Description:        Scan all available channel and look for a LFG broadcast
 * Function Busy times:         50 ms per channel scanned
 * Function Return Labels:      0
 *
 * Function Returns:            0 - no LFG found
 *                              ID - node broadcasting LFG with <ID>
**/
int mcu_function_scan_lfg(struct Node* nodes,
                          int node_count,
                          int id,
                          int group_max,
                          int debug) {
    int own_function_number = 1;

    for (int i = 0; i < node_count; i++) {
        if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
            update_signal(nodes, id, i, debug);
        }
    }
    if (nodes[id].active_channel < 64) {        // go to next channel if less than max_channels (using 64 for now)
        nodes[id].active_channel++;
    }
    else {                                      // if done scanning, return to main
        nodes[id].active_channel = 0;
        mcu_return(nodes, id, own_function_number, 0);
    }
    return 0;
}

/**
 * Function Number:             2
 * Function Name:               broadcast_lfg
 * Function Description:        Broadcasts LFG message 
 * Function Busy times:         0
 * Function Return Labels:      2
 
 * Return Label 0 returns from: 3 (find_clear_channel)
 * Return Lable 0 reason:       Get return value of first available channel for
                                broadcasting (if any)

 * Return Label 1 returns from: 5 (transmit_message_begin)
 * Return Lable 1 reason:       Check for successful transmission of LFG message

 * Return Label 2 returns from: 6 (transmit_message_complete)
 * Return Lable 2 reason:       Turn off transmit

 * Function Returns:            0 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_broadcast_lfg(struct Node* nodes,
                               int id,
                               int group_max,
                               int debug) {
    int own_function_number = 2;
                                
    if (nodes[id].return_stack->returning_from == 3) {
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value > 0) {
            // If clear channel was found, broadcast LFG on it
            snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), "LFG");
            mcu_call(nodes, id, own_function_number, 1, 5);
            return 0;
        }
        else {
            // No clear channel was found, notify caller
            mcu_return(nodes, id, own_function_number, 0);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 5) {
        // Returning from transmit_message_begin
        // No error checking for now
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 2, 6);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 6) {
        // Returning from transmit_message_complete
        // No error checking for now, just return channel number
        rs_pop(&nodes[id].return_stack);
        mcu_return(nodes, id, own_function_number, nodes[id].active_channel);
        return 0;
    }
    else {  
        // Not returning from a call (first entry)
        mcu_call(nodes, id, own_function_number, 0, 3);
    }
    return 0;
}

/**
 * Function Number:             3
 * Function Name:               find_clear_channel
 * Function Description:        Listens for any node transmitting and finds first
                                available free channel
 * Function Busy time:          0
 * Function Return Labels:      1
 
 * Return Label 0 returns from: 4 (check_channel_busy)
 * Return Lable 0 reason:       See if there is any activity on selected channel

 * Function Returns:            0 - no clear channels
 *                              channel - first available free channel
**/
int mcu_function_find_clear_channel(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    int debug) {
    int own_function_number = 3;

    // check for return
    if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == 1) {
            // Channel was busy, go to next, unless at last channel
            if (nodes[id].active_channel == 64) {
                nodes[id].active_channel = 0;
                mcu_return(nodes, id, own_function_number, 0);
                return 0;
            }
            else {
                nodes[id].active_channel++;
                mcu_call(nodes, id, own_function_number, 0, 4);
            }
        }
        else {
            // Channel was free, return to caller
            mcu_return(nodes, id, own_function_number, nodes[id].active_channel);
            return 0;
        }
    }
    else {
        // Not returning from a call (first entry)
        nodes[id].active_channel = 0;
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;
}

/**
 * Function Number:             4
 * Function Name:               check_channel_busy
 * Function Description:        Checks to see if there are any transmissions on 
                                selected channel
 * Function Busy time:          50ms 
 * Function Return Labels:      0

 * Function Returns:            0 - channel free
 *                              1 - channel busy
**/
int mcu_function_check_channel_busy(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    int debug) {
    int own_function_number = 4;
    
    for (int i = 0; i < node_count; i++) {
        if (i != id) {                  // don't check own id
            if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active) {
                mcu_return(nodes, id, own_function_number, 1);
                return 0;
            }
        }
    }
    mcu_return(nodes, id, own_function_number, 0);
    return 0;    
}

/**
 * Function Number:             5
 * Function Name:               transmit_message_begin
 * Function Description:        Transmits message specified in node.send_packet
 * Function Busy time:          0 
 * Function Return Labels:      0

 * Function Returns:            0 - transmit error
 *                              1 - transmit successful
**/
int mcu_function_transmit_message_begin(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           int debug) {
    int own_function_number = 5;
    
    nodes[id].transmit_active = 1;

    mcu_return(nodes, id, own_function_number, 1);
    return 0;    
}

/**
 * Function Number:             6
 * Function Name:               transmit_message_complete
 * Function Description:        Turns off transmit after specified duration
 * Function Busy time:          2.0 
 * Function Return Labels:      0

 * Function Returns:            0 - transmit error
 *                              1 - transmit successful
**/
int mcu_function_transmit_message_complete(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           int debug) {
    int own_function_number = 6;
    
    nodes[id].transmit_active = 0;

    mcu_return(nodes, id, own_function_number, 1);
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