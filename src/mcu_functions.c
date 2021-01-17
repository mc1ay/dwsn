/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller functions
 *
 * @author  Mitchell Clay
 * @date    1/1/2021
**/

#include "mcu_functions.h"

/**
 * Function Number:             0
 * Function Name:               main
 * Function Description:        Main Loop of microcontroller emulation routine
 * Function Busy times:         None
 * Function Return Labels:      4
  
 * Return Label 0 returns from: 2 (broadcast_lfg)
 * Return Label 0 reason:       Check to see if LFG was successfully broadcast on
                                an open channel

 * Return Label 1 returns from: 1 (scan_lfg)
 * Return Label 1 reason:       Check to see if scan function finds a looking for 
                                group broadcast message

 * Return Label 2 returns from: 9 (respond_lfg)
 * Return Label 2 reason:       Respond to LFG broadcast

 * Return Label 3 returns from: 7 (sleep)
 * Return Label 3 reason:       Once in sleep function, keep repeating, for now 

 * Return Label 4 returns from: 9 (scan_lfg_responses)
 * Return Label 4 reason:       Handle LFG responses heard on active channel                                                                 

 * Function Returns:            nothing
**/
int mcu_function_main(struct Node* nodes,
                     int node_count,
                     int id,
                     int group_max,
                     int channels,
                     int debug) {
    int own_function_number = 0;

    if (nodes[id].return_stack->returning_from == 2) {
        // returning from LFG broadcast, listen for replies for specified time
        // TO-DO: make scan time a parameter later
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value < 0) {    
            printf("No clear channels found\n");    
        }
        else {
            // scan for LFG reply packets
            mcu_call(nodes, id, own_function_number, 4, 10);
            return 0;
        }

    }

    if (nodes[id].return_stack->returning_from == 1) {
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value < 0) {    
            printf("Something is wrong\n");    
            // something is wrong
        }
        else {
            // respond to LFG broadcast from node with strongest signal
            int strongest_node_id = -1;
            double strongest_signal = -1000000; // using large negative for now (update later)

            // find strongest signal broadcasting LFG
            for (int i = 0; i < channels; i++) {
                if (nodes[id].tmp_lfg_chans[i] != -1) {
                    if (nodes[id].received_signals[i] > strongest_signal) {
                        strongest_node_id = i;
                    }
                }
            }

            // set active channel to same channel as strongest LFG broadcaster
            nodes[id].active_channel = strongest_node_id;

            // call respond_lfg
            mcu_call(nodes, id, own_function_number, 2, 9);
            return 0;
        }
    }

    else if (nodes[id].return_stack->returning_from == 9) {
        // for now, periodically run another scan
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 3, 1);
        return 0;
    }

    else {
        // First time entering main
        // Broadcast from first two nodes, others listen
        if (id < 2) {
            nodes[id].active_channel = 3 * id % 2;
            mcu_call(nodes, id, own_function_number, 0, 2);
            return 0; 
        }
        else {
            mcu_call(nodes, id, own_function_number, 1, 1);
            return 0;
        }    
    }
    return 0;
}

/**
 * Function Number:             1
 * Function Name:               scan_lfg
 * Function Description:        Scan all available channel and look for a LFG broadcast
 * Function Busy times:         0
 * Function Return Labels:      2
 *
 * Return Label 0 returns from: 4 (check_channel_busy)
 * Return Label 0 reason:       See if there is any activity on selected channel
 * Return Label 1 returns from: 7 (receive)
 * Return Label 1 reason:       Get incoming packet
 *
 * Function Returns:            -1 - no LFG found
 *                              ID - node broadcasting LFG with <ID>
**/
int mcu_function_scan_lfg(struct Node* nodes,
                          int node_count,
                          int id,
                          int group_max,
                          int channels,
                          int debug) {
    int own_function_number = 1;

    if (nodes[id].return_stack->returning_from == 7) {
        // Returning from receive function
        // Return value is sending node ID
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == -1) {
            // collision detected try again 
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        if (return_value == -2) {
            // Nothing heard try again
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            // Check for LFG
            if (strcmp(nodes[return_value].send_packet, "LFG") == 0) {
                // Found LFG packet, add to LFG tmp array
                // Put sending node id into correct channel slot of array
                nodes[id].tmp_lfg_chans[nodes[id].active_channel] = return_value;
            }
            // Keep scanning if not at last channel
            if (nodes[id].active_channel == 16) {
            // If at last channel, return to main
                nodes[id].active_channel = 0;
                mcu_return(nodes, id, own_function_number, return_value);
                return 0;
            }
            else {
                nodes[id].active_channel++;
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
            }
        }
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            // Didn't hear anything, go to next channel
            if (nodes[id].active_channel == 16) {
                // If at last channel, return to main
                nodes[id].active_channel = 0;
                mcu_return(nodes, id, own_function_number, return_value);
                return 0;
            }
            else {
                nodes[id].active_channel++;
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
            }
        }
    }
    else {
        // Not returning from a call (first entry)
        // Start at first channel
        nodes[id].active_channel = 0;
        // Initialize LFG tmp array before scanning
        for (int i = 0; i < channels; i++) {
            nodes[id].tmp_lfg_chans[i] = -1;
        }
        // Check if first channel is busy
        mcu_call(nodes, id, own_function_number, 0, 4);
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
 * Return Label 0 reason:       Get return value of first available channel for
                                broadcasting (if any)

 * Return Label 1 returns from: 5 (transmit_message_begin)
 * Return Label 1 reason:       Check for successful transmission of LFG message

 * Return Label 2 returns from: 6 (transmit_message_complete)
 * Return Label 2 reason:       Turn off transmit

 * Function Returns:            -1 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_broadcast_lfg(struct Node* nodes,
                               int id,
                               int group_max,
                               double* current_time,
                               int debug) {
    int own_function_number = 2;
                                
    if (nodes[id].return_stack->returning_from == 3) {
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value >= 0) {
            // If clear channel was found, broadcast LFG on it
            snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), "LFG");
            mcu_call(nodes, id, own_function_number, 1, 5);
            return 0;
        }
        else {
            // No clear channel was found, notify caller
            mcu_return(nodes, id, own_function_number, -1);
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
        // No error checking for now, just transmit until timer expired
        rs_pop(&nodes[id].return_stack);
        // check time
        if (nodes[id].tmp_start_time + 2.0 < *current_time) {
            // time expired stop listening for replies, return to main
            // after resetting timer
            nodes[id].tmp_start_time = 999999.9;   // just using a large value for now
            mcu_return(nodes, id, own_function_number, nodes[id].active_channel);
            return 0;  
        }
        else {
            // timer not expired, keep transmitting
            mcu_call(nodes, id, own_function_number, 1, 5);
        }
        return 0;
    }
    else {  
        // Not returning from a call (first entry)
        nodes[id].tmp_start_time = *current_time;
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
 * Return Label 0 reason:       See if there is any activity on selected channel

 * Function Returns:            -1 - no clear channels
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
            if (nodes[id].active_channel == 16) {
                nodes[id].active_channel = 0;
                mcu_return(nodes, id, own_function_number, -1);
                return 0;
            }
            else {
                nodes[id].active_channel++;
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
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
            if (nodes[id].active_channel == nodes[i].active_channel && nodes[i].transmit_active == 1) {
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

/**
 * Function Number:             7
 * Function Name:               receive
 * Function Description:        MCU gets data from channel
 * Function Busy time:          0   needs to be updated later to match transmit time
 * Function Return Labels:      0

 * Function Returns:           -2 - nothing received 
 *                             -1 - collision
 *                             ID - received data from node <id>
**/
int mcu_function_receive(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           int debug) {
    int own_function_number = 7;
    int signals_detected = 0;
    int transmitting_node = -1;

    for (int i = 0; i < node_count; i++) {
        if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
            update_signal(nodes, id, i, debug);
            signals_detected++;
            if (signals_detected > 1) {
            }
            transmitting_node = i;
        }
    }
    if (signals_detected > 1) {
        mcu_return(nodes, id, own_function_number, -1);
        return 0;
    }
    else if (signals_detected == 0) {
        mcu_return(nodes, id, own_function_number, -2);
        return 0;
    }
    mcu_return(nodes, id, own_function_number, transmitting_node);
    return 0;    
}

/**
 * Function Number:             8
 * Function Name:               sleep
 * Function Description:        MCU does nothing (add power reduction later)
 * Function Busy time:          1.0 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_sleep(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           int debug) {
    int own_function_number = 8;
    
    mcu_return(nodes, id, own_function_number, 0);
    return 0;    
}

/**
 * Function Number:             9
 * Function Name:               respond_lfg
 * Function Description:        Respond to LFG broadcast
 * Function Busy times:         None
 * Function Return Labels:      4

 * Return Label 0 returns from: 4 (check_channel_busy)
 * Return Label 0 reason:       Make sure nothing is transmitting on channel before 
                                responding to LFG

 * Return Label 1 returns from: 5 (transmit_message_begin)
 * Return Label 1 reason:       Check for successful transmission of LFG message

 * Return Label 2 returns from: 6 (transmit_message_complete)
 * Return Label 2 reason:       Turn off transmit

 * Return Label 3 returns from: 11 (random wait)
 * Return Label 3 reason:       waiting period to minimize collisions

 * Function Returns:            -1 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_respond_lfg(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           int debug) {
    int own_function_number = 9;
    

    if (nodes[id].return_stack->returning_from == 11) {
        // Random wait is over
        rs_pop(&nodes[id].return_stack);
        // call transmit function
        mcu_call(nodes, id, own_function_number, 1, 5);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == 1) {
            // channel was busy, try again
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;
        }
        else if (return_value == 0) {
            snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), "LFG-R");
            // add random wait value before transmitting to minimize collisions
            mcu_call(nodes, id, own_function_number, 3, 11);
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
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;   
}

/**
 * Function Number:             10
 * Function Name:               scan_lfg_responses
 * Function Description:        Listens for LFG response packets on active channel for specific time
 * Function Busy time:          0 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_scan_lfg_responses(struct Node* nodes,
                                           int node_count,
                                           int id,
                                           double* current_time,
                                           int group_max,
                                           int debug) {
    int own_function_number = 10;

    if (nodes[id].return_stack->returning_from == 12) {
        // Returning from ACK transmit
        // No return checking for now
        // Just keep scanning
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 0, 4);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 7) {
        // Returning from receive function
        // Return value is sending node ID
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == -1) {
            // collision detected try again 
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        if (return_value == -2) {
            // Nothing heard try again
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            // Check for LFG
            if (strcmp(nodes[return_value].send_packet, "LFG-R") == 0) {
                // Found LFG-R packet, add node to group
                // TO-DO: add response packet
                int available_slot = -1;
                int i = 0;
                do {
                    if (nodes[id].group_list[i] == return_value) {
                        // already in group, just return for now
                        // TO-DO: response to this
                        nodes[id].tmp_start_time = 999999.9;   // just using a large value for now
                        mcu_return(nodes, id, own_function_number, 0);
                    }
                    if (nodes[id].group_list[i] == -1) {
                        available_slot = i;
                    }
                    i++;
                }
                while (i < group_max && available_slot == -1);
 
                if (available_slot == -1) {
                    printf("Group full!\n");
                    // group is full (TO-DO, respond to this)
                    // for now, return to main
                    nodes[id].tmp_start_time = 999999.9;   // just using a large value for now
                    mcu_return(nodes, id, own_function_number, 0);
                    return 0;
                }
                else {
                    // add node to group list
                    printf("node %d added node %d to group\n", id, return_value);
                    nodes[id].group_list[available_slot] = return_value;
                    // send ACK
                    nodes[id].dest_node = return_value;
                    mcu_call(nodes, id, own_function_number, 0, 12);
                    return 0;                    
                }
            }
            else {
                // Not LFG-R packet, keep listening
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
            }
        }
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // check time
        if (nodes[id].tmp_start_time + 2.0 < *current_time) {
            // time expired stop listening for replies, return to main
            nodes[id].tmp_start_time = 999999.9;   // just using a large value for now
            mcu_return(nodes, id, own_function_number, 0);
        return 0;  
        }
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
    }
    else {
        // Not returning from a call (first entry)
        // set start_time and check for activity on active channel
        nodes[id].tmp_start_time = *current_time;
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;    
}

/**
 * Function Number:             11
 * Function Name:               random_wait
 * Function Description:        sets busy time to random value
 * Function Busy time:          random 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_random_wait(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    double time_resolution,
                                    int debug) {    
    int own_function_number = 11;
    // for now this does nothing, busy_time is set in mcu_run_function()
    mcu_return(nodes, id, own_function_number, 0);

    return 0;    
}

/**
 * Function Number:             12
 * Function Name:               lfgr_send_ack
 * Function Description:        replies to sender that lfg-r was received
 * Function Busy time:          0 
 * Function Return Labels:      3

 * Return Label 0 returns from: 4 (check_channel_busy)
 * Return Label 0 reason:       Make sure nothing is transmitting on channel before 
                                responding to LFG

 * Return Label 1 returns from: 5 (transmit_message_begin)
 * Return Label 1 reason:       Check for successful transmission of LFG message

 * Return Label 2 returns from: 6 (transmit_message_complete)
 * Return Label 2 reason:       Turn off transmit

 * Function Returns:            -1 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_lfgr_send_ack(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    int debug) {    
    int own_function_number = 12;

    if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == 1) {
            // channel was busy, try again
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;
        }
        else if (return_value == 0) {
            snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), 
                     "LFG-R ACK %d", nodes[id].dest_node);
            mcu_call(nodes, id, own_function_number, 1, 5);
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
        printf("Node %d sent %s\n", id, nodes[id].send_packet);
        rs_pop(&nodes[id].return_stack);
        mcu_return(nodes, id, own_function_number, nodes[id].active_channel);
        return 0;
    }
    else {
        // Not returning from a call (first entry)
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;   
}

/**
 * Function Number:             13
 * Function Name:               lfgr_get_ack
 * Function Description:        listens for lfgr_ack packet 
 * Function Busy time:          random 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_lfgr_get_ack(struct Node* nodes,
                                    int node_count,
                                    int id,
                                    double* current_time,
                                    int debug) {    
    int own_function_number = 13;
    // for now this does nothing, busy_time is set in mcu_run_function()
    mcu_return(nodes, id, own_function_number, 0);

    return 0;    
}