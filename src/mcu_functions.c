/**
 * @file    mcu_emulation.c
 * @brief   Microcontroller functions
 *
 * @author  Mitchell Clay
 * @date    1/1/2021
**/

#include "mcu_functions.h"
#include "messages.h"
#include "state.h"
#include "timers.h"

extern struct Settings settings;
extern struct State state;

/**
 * Function Number:             0
 * Function Name:               main
 * Function Description:        Main Loop of microcontroller emulation routine
 * Function Busy times:         None
 * Function Return Labels:      7
  
 * Return Label 0 returns from: 2 (broadcast_lfg)
 * Return Label 1 returns from: 1 (scan_lfg)
 * Return Label 2 returns from: 9 (respond_lfg)
 * Return Label 3 returns from: 8 (sleep)
 * Return Label 4 returns from: 10 (scan_lfg_responses)
 * Return Label 5 returns from: 8 (sleep)
 * Return Label 6 returns from: 8 (sleep)
 * Return Label 7 returns from: 14 (group_cycle_start)
 * Return Label 8 returns from: 15 (sensor_data_send)
 * Return Label 9 returns from: 16 (sensor_data_recv)


 * Function Returns:            nothing
**/
int mcu_function_main(struct Node* nodes, int id) {
    int own_function_number = 0;

    if (nodes[id].return_stack->returning_from == 1) {
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);

        // see if group cycle timer has expired
        if (nodes[id].group_cycle_start + settings.group_cycle_interval <= state.current_cycle) {
            // Timer expired
            mcu_call(nodes, id, own_function_number, 7, 14);
            return 0;
        }

        if (return_value < 0) {    
            printf("Something is wrong\n");    
            // something is wrong
        }
        else {
            // respond to LFG broadcast from node with strongest signal
            int strongest_node_id = -1;
            double strongest_signal = 1; // using 1 for testing
            
            // if debugging, print nodes found
            if (settings.debug) {
                printf("After scanning node %d heard broadcasts from: \n", id);
            }
    
            // find strongest signal broadcasting LFG
            for (int i = 0; i < settings.channels; i++) {
                if (nodes[id].tmp_lfg_chans[i] != -1) {
                    if (settings.debug) {
                        printf("  Node %d (%f dBM)\n", nodes[id].tmp_lfg_chans[i], 
                               nodes[id].received_signals[nodes[id].tmp_lfg_chans[i]]);
                    }
                    if (nodes[id].received_signals[nodes[id].tmp_lfg_chans[i]] > strongest_signal) {
                        strongest_node_id = nodes[id].tmp_lfg_chans[i];
                        strongest_signal = nodes[id].received_signals[nodes[id].tmp_lfg_chans[i]];
                    }
                }
            }
            // if no available nodes, scan again
            if (strongest_node_id == -1) {
                if (settings.debug) {
                    printf("Node %d didn't hear any LFG messages, scanning again\n", id);
                }
                mcu_call(nodes, id, own_function_number, 1, 1);
                return 0;
            }
            if (settings.debug) {
            printf("Node %d will attempt to send LFG-R to node %d on channel %d\n",
                    id, strongest_node_id, nodes[strongest_node_id].active_channel);
            }
            
            // set active channel to same channel as strongest LFG broadcaster
            nodes[id].active_channel = nodes[strongest_node_id].active_channel;

            // set destination node id
            nodes[id].dest_node = strongest_node_id;

            // call respond_lfg
            mcu_call(nodes, id, own_function_number, 2, 9);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 2) {
        // returning from LFG broadcast, listen for replies for specified time
        // TO-DO: make scan time a parameter later
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value < 0) {    
            printf("No clear channels found\n");    
        }
        else {
            // scan for LFG reply packets
            if (settings.debug) {
                printf("Node %d listening for LFG replies\n", id);
            }
            mcu_call(nodes, id, own_function_number, 4, 10);
            return 0;
        }

    }
    else if (nodes[id].return_stack->returning_from == 8) {
        int label = nodes[id].return_stack->return_to_label;
        rs_pop(&nodes[id].return_stack);

        // see if group cycle timer has expired
        if (nodes[id].group_cycle_start + settings.group_cycle_interval <= state.current_cycle) {
            // Timer expired
            mcu_call(nodes, id, own_function_number, 7, 14);
            return 0;
        }

        if (label == 3) {
            // non-broadcaster, send sensor data
            // TO-DO, add return for this
            mcu_call(nodes, id, own_function_number, 8, 15);
            return 0;
        }

        // if timer not expired, stay asleep
        mcu_call(nodes, id, own_function_number, 5, 8);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 9) {
        // for now, just go to sleep
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 3, 8);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 10) {
        // listen for DATA packets
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 9, 16);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 14) {
        rs_pop(&nodes[id].return_stack);
        if (nodes[id].broadcaster == 1) {
            mcu_call(nodes, id, own_function_number, 0, 2);
            return 0; 
        }
        else {
            mcu_call(nodes, id, own_function_number, 1, 1);
            return 0;
        }
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 10) {
        // listen for DATA packets
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 9, 16);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 15) {
        rs_pop(&nodes[id].return_stack);

        // see if group cycle timer has expired
        if (nodes[id].group_cycle_start + settings.group_cycle_interval <= state.current_cycle) {
            // Timer expired
            mcu_call(nodes, id, own_function_number, 7, 14);
            return 0;
        }
        else {
            // keep transmitting DATA packets 
            mcu_call(nodes, id, own_function_number, 8, 15);
        }
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 16) {
        rs_pop(&nodes[id].return_stack);
        // see if group cycle timer has expired
        if (nodes[id].group_cycle_start + settings.group_cycle_interval <= state.current_cycle) {
            // Timer expired
            mcu_call(nodes, id, own_function_number, 7, 14);
            return 0;
        }
        // keep listening
        mcu_call(nodes, id, own_function_number, 9, 16);
        return 0;
    }
    else {
        // First time entering main
        // Start group cycle
        mcu_call(nodes, id, own_function_number, 7, 14);
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
 * Return Label 1 returns from: 7 (receive)
 *
 * Function Returns:            -1 - no LFG found
 *                              ID - node broadcasting LFG with <ID>
**/
int mcu_function_scan_lfg(struct Node* nodes, int id) {
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
            // Mark channel as scanned
            nodes[id].tmp_scanned_chans[nodes[id].active_channel] = 1;

            // Check for LFG
            char* token;
            char incoming_buffer[256];

            strncpy(incoming_buffer, nodes[return_value].send_packet, 256);

            token = strtok(incoming_buffer, " ");
            if (token != NULL) {
                token = strtok(NULL, " ");
            }
            if (token != NULL) {
                token = strtok(NULL, " ");
            }
            if (strcmp(token, "LFG") == 0) {
                // Found LFG packet, add to LFG tmp array
                // Put sending node id into correct channel slot of array
                nodes[id].tmp_lfg_chans[nodes[id].active_channel] = return_value;
            }
   
            // Keep scanning if not at last channel
            // See how many unscanned channels are left
            int unscanned_channel_count = 0;
            for (int i = 0; i < settings.channels; i++) {
                if (nodes[id].tmp_scanned_chans[i] == 0) {
                    unscanned_channel_count++;
                }
            }
            if (unscanned_channel_count == 0) {
                // If all channels scanned, clear array and scan again 
                // Initialize tmp_scanned_chans array
                for (int i = 0; i < settings.channels; i++) {
                    nodes[id].tmp_scanned_chans[i] = 0;
                }
                // Pick random start channel
                nodes[id].active_channel = rand() % settings.channels;
                // Check if first channel is busy
                mcu_call(nodes, id, own_function_number, 0, 4); 
                return 0;
            }
            else {
                // Make array of channels that haven't been scanned
                int unscanned_chans[unscanned_channel_count];
                int channel = 0;
                for (int i = 0; i < unscanned_channel_count; i++) {
                    while (nodes[id].tmp_scanned_chans[channel] == 1) {
                        channel++;
                    }
                    unscanned_chans[i] = channel;
                    channel++;
                }
                // Pick an unscanned channel at random to try next
                nodes[id].active_channel = unscanned_chans[rand() % unscanned_channel_count];
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
            }
        }
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);

        // Check cycle timer
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            mcu_return(nodes, id, own_function_number, 0);
            return 0;
        }
    
        // Mark channel as scanned
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            // Mark channel as scanned
            nodes[id].tmp_scanned_chans[nodes[id].active_channel] = 1;

            // Didn't hear anything, go to next channel
            // See how many unscanned channels are left
            int unscanned_channel_count = 0;
            for (int i = 0; i < settings.channels; i++) {
                if (nodes[id].tmp_scanned_chans[i] == 0) {
                    unscanned_channel_count++;
                }
            }
            if (unscanned_channel_count == 0) {
                // If all channels scanned, clear array and scan again 
                // Initialize tmp_scanned_chans array
                for (int i = 0; i < settings.channels; i++) {
                    nodes[id].tmp_scanned_chans[i] = 0;
                }
                // Pick random start channel
                nodes[id].active_channel = rand() % settings.channels;
                // Check if first channel is busy
                mcu_call(nodes, id, own_function_number, 0, 4); 
                return 0;
            }
            else {
                // Make array of channels that haven't been scanned
                int unscanned_chans[unscanned_channel_count];
                int channel = 0;
                for (int i = 0; i < unscanned_channel_count; i++) {
                    while (nodes[id].tmp_scanned_chans[channel] == 1) {
                        channel++;
                    }
                    unscanned_chans[i] = channel;
                    channel++;
                }

                // Pick an unscanned channel at random to try next
                nodes[id].active_channel = unscanned_chans[rand() % unscanned_channel_count];
                mcu_call(nodes, id, own_function_number, 0, 4);
                return 0;
            }
        }
    }
    else {
        // First time entering function
        // Create cycle timer
        if (settings.debug) {
            printf("Node %d created cycle timer for function %d\n", id, own_function_number);
        }
        nodes[id].timers = 
            cycle_timer_create(nodes[id].timers, own_function_number, 0, state.current_cycle, 1000);

        // Initialize LFG tmp array before scanning
        for (int i = 0; i < settings.channels; i++) {
            nodes[id].tmp_lfg_chans[i] = -1;
        }
        // Initialize tmp_scanned_chans array
        for (int i = 0; i < settings.channels; i++) {
            nodes[id].tmp_scanned_chans[i] = 0;
        }
        // Pick random start channel
        nodes[id].active_channel = rand() % settings.channels;
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

 * Return Label 3 returns from: 11 (random_wait)
 * Return Label 3 reason:       Wait before scanning

 * Return Label 4 returns from: 5 (transmit_message_begin)
 * Return Label 4 reason:       Subsequent transmits

 * Function Returns:            -1 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_broadcast_lfg(struct Node* nodes, int id) {
    int own_function_number = 2;

    if (nodes[id].return_stack->returning_from == 11) {
        // Returned from random wait
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 0, 3);
        return 0;
    }                            
    else if (nodes[id].return_stack->returning_from == 3) {
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value >= 0) {
            // If clear channel was found, broadcast LFG on it
            snprintf(nodes[id].send_packet, 256, "N-ALL N-%d LFG", id);
        if (settings.debug) {
            printf("Node %d broadcasting LFG on channel %d\n", id, nodes[id].active_channel);
        }
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
        rs_pop(&nodes[id].return_stack);
        // Check cycle timer
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            // time expired, stop transmitting
            mcu_call(nodes, id, own_function_number, 4, 6);
            return 0;
        }
        else {
            // No error checking for now, just transmit until timer expired
            mcu_call(nodes, id, own_function_number, 4, 5);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 6) {
        // Returning from transmit_message_complete
        if (settings.debug) {
            printf("Node %d stopped broadcasting LFG\n", id);
        }
        // No error checking for now
        rs_pop(&nodes[id].return_stack);
        // Return to main
        mcu_return(nodes, id, own_function_number, nodes[id].active_channel);

        return 0;
    }
    else {  
        // Not returning from a call
        // Create cycle timer
        if (settings.debug) {
            printf("Node %d created cycle timer for function %d\n", id, own_function_number);
        }
        nodes[id].timers = 
            cycle_timer_create(nodes[id].timers, own_function_number, 0, state.current_cycle, 1000);  
        
        mcu_call(nodes, id, own_function_number, 3, 11);
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
int mcu_function_find_clear_channel(struct Node* nodes, int id) {
    int own_function_number = 3;

    // check for return
    if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        // Mark channel as scanned
        nodes[id].tmp_scanned_chans[nodes[id].active_channel] = 1;

        // Check return value
        if (return_value == 1) {
            // Channel was busy, find another unless all are busy
            // First, see how many channels haven't been checked
            int unscanned_channel_count = 0;
            for (int i = 0; i < settings.channels; i++) {
                if (nodes[id].tmp_scanned_chans[i] == 0) {
                    unscanned_channel_count++;
                }
            }
            if (unscanned_channel_count == 0) {
                // All channels have been scanned
                mcu_return(nodes, id, own_function_number, -1);
                return 0;
            }
            else {
                // Make array of channels that haven't been scanned
                int unscanned_chans[unscanned_channel_count];
                int channel = 0;
                for (int i = 0; i < unscanned_channel_count; i++) {
                    while (nodes[id].tmp_scanned_chans[channel] == 1) {
                        channel++;
                    }
                    unscanned_chans[i] = nodes[id].tmp_scanned_chans[channel];
                    channel++;
                }
                // Pick an unscanned channel at random to try next
                nodes[id].active_channel = unscanned_chans[rand() % unscanned_channel_count];
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
        // Initialize tmp_scanned_chans array
        for (int i = 0; i < settings.channels; i++) {
            nodes[id].tmp_scanned_chans[i] = 0;
        }
        nodes[id].active_channel = rand() % settings.channels;
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
int mcu_function_check_channel_busy(struct Node* nodes, int id) {
    int own_function_number = 4;
    
    for (int i = 0; i < settings.node_count; i++) {
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
int mcu_function_transmit_message_begin(struct Node* nodes, int id) {
    int own_function_number = 5;
    if (nodes[id].transmit_active == 0) {
        nodes[id].transmit_active = 1;
    }
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
int mcu_function_transmit_message_complete(struct Node* nodes, int id) {
    int own_function_number = 6;
    
    // Turn off transmit
    nodes[id].transmit_active = 0;

    // Erase send packet
    //for (int i = 0; i < sizeof(nodes[id].send_packet); i++) {
    //    nodes[id].send_packet[i] = '\0';
    //}

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
int mcu_function_receive(struct Node* nodes, int id) {
    int own_function_number = 7;
    int signals_detected = 0;
    int transmitting_node = -1;

    for (int i = 0; i < settings.node_count; i++) {
        if (i != id) {          // Don't check own ID
            if (nodes[i].transmit_active && nodes[id].active_channel == nodes[i].active_channel) {
                update_signal(nodes, id, i);
                signals_detected++;
                if (signals_detected > 1) {
                }
                transmitting_node = i;
            }
        }
    }
    if (signals_detected > 1) {
        if (settings.debug) {
            printf("Node %d detected collision\n", id);
        }
        state.collisions++;
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
int mcu_function_sleep(struct Node* nodes, int id) {
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

 * Return Label 4 returns from: 11 (random wait)
 * Return Label 4 reason:       re-transmit wait

 * Function Returns:            -1 - no clear channels
 *                              channel - sent LFG on <channel>
**/
int mcu_function_respond_lfg(struct Node* nodes, int id) {
    int own_function_number = 9;

    if (nodes[id].return_stack->returning_from == 13) {
        // See if ACK was received
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);

        // Check cycle timer
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            mcu_return(nodes, id, own_function_number, 0);
            return 0;
        }

        if (return_value == 1) {
            // ACK was received, return to caller
            mcu_return(nodes, id, own_function_number, nodes[id].active_channel);
            return 0;
        }
        else {
            // ACK not received try again
            // later, check to see if group was full or other error
            mcu_call(nodes, id, own_function_number, 4, 11);
            return 0; 
        }
    }
    else if (nodes[id].return_stack->returning_from == 11) {
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
    
        // Check cycle timer
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            mcu_return(nodes, id, own_function_number, 0);
            return 0;
        }
    
        if (return_value == 1) {
            // channel was busy, try again
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;
        }
        else if (return_value == 0) {
            snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), "N-%d N-%d LFG-R", 
                     nodes[id].dest_node, id);
            // add random wait value before transmitting to minimize collisions
            mcu_call(nodes, id, own_function_number, 3, 11);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 5) {
        // Returning from transmit_message_begin
        // No error checking for now
        if (settings.debug) {
            printf("Node %d sent \"%s\" on channel %d\n", id, 
                   nodes[id].send_packet, nodes[id].active_channel);
        }
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 2, 6);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 6) {
        // Returning from transmit_message_complete
        // No error checking for now, just check for ACK
        rs_pop(&nodes[id].return_stack);
        // Check for ACK
        mcu_call(nodes, id, own_function_number, 4, 13);
        return 0;
    }
    else {
        // Not returning from a call (first entry)
        // Create cycle timer
        nodes[id].timers = 
            cycle_timer_create(nodes[id].timers, own_function_number, 0, state.current_cycle, 1000);

        // Check for activity on channel    
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
int mcu_function_scan_lfg_responses(struct Node* nodes, int id) {
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
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        if (return_value == -2) {
            // Nothing heard try again
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        else {
            // Check for LFG-R
            char* token;
            char incoming_buffer[256];

            strncpy(incoming_buffer, nodes[return_value].send_packet, 256);

            token = strtok(incoming_buffer, " ");
            char my_id[6];
            char sender_id[6];
            snprintf(my_id, 6, "N-%d", id);
            
            // Extract dest and src node IDs from packet
            if (strcmp(token, my_id) == 0) {
                token = strtok(NULL, " ");
                strncpy(sender_id, token, 6);

                // Check if third token is "LFG-R"
                token = strtok(NULL, " ");
                if (strcmp(token, "LFG-R") == 0) {
                    if (settings.debug) {
                        printf("Node %d heard 'LFG-R' from node %d\n", id, return_value);
                    }
                    // Found LFG-R packet, add node to group
                    int available_slot = -1;
                    int i = 0;
                    do {
                        if (nodes[id].group_list[i] == return_value) {
                            // already in group, re-send ACK
                            nodes[id].dest_node = return_value;
                            mcu_call(nodes, id, own_function_number, 0, 12);
                            return 0; 
                        }
                        if (nodes[id].group_list[i] == -1) {
                            available_slot = i;
                        }
                        i++;
                    }
                    while (i < settings.group_max - 1 && available_slot == -1);
    
                    if (available_slot == -1) {
                        // group is full (TO-DO, respond to this)
                        // for now, return to main
                        nodes[id].tmp_start_time = FLT_MAX;
                        mcu_return(nodes, id, own_function_number, 0);
                        return 0;
                    }
                    else {
                        // add node to group list
                        if (settings.debug) {
                            printf("node %d added node %d to group\n", id, return_value);
                        }
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
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        // check time
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            // time expired stop listening for replies, return to main
            if (settings.debug) {
                printf("Node %d stopped listening for LFG-R\n", id);
            }
            mcu_return(nodes, id, own_function_number, 0);
            return 0;  
        }
        // time not expired, continue
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;            
        }
    }
    else {
        // Not returning from a call (first entry)
        // set start_time and check for activity on active channel
        if (settings.debug) {
            printf("Node %d listening for LFG-R packets on channel %d\n", id, nodes[id].active_channel);
        }

        // Create cycle timer
        if (settings.debug) {
            printf("Node %d created cycle timer for function %d\n", id, own_function_number);
        }
        nodes[id].timers = 
            cycle_timer_create(nodes[id].timers, own_function_number, 0, state.current_cycle, 1000);  

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
int mcu_function_random_wait(struct Node* nodes, int id) {    
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

 * Function Returns:            0 (update later)
**/
int mcu_function_lfgr_send_ack(struct Node* nodes, int id) {    
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
                     "N-%d N-%d ACK LFG-R", nodes[id].dest_node, id);
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
        if (settings.debug) {
            printf("Node %d sent \"%s\" on channel %d\n", id, 
                   nodes[id].send_packet, nodes[id].active_channel);
        }
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

 * Function Returns:            1 - ACK received
 *                              0 - Nothing heard
**/
int mcu_function_lfgr_get_ack(struct Node* nodes, int id) {    
    int own_function_number = 13;

    if (nodes[id].return_stack->returning_from == 7) {
        // Returning from receive function
        // Return value is sending node ID
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == -1) {
            // collision detected try again 
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        if (return_value == -2) {
            // Nothing heard try again
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        else {
            // Check for LFG-R
            char* token;
            char incoming_buffer[256];

            strncpy(incoming_buffer, nodes[return_value].send_packet, 256);

            token = strtok(incoming_buffer, " ");
            char my_id[6];
            char sender_id[6];
            snprintf(my_id, 6, "N-%d", id);
            
            // Extract dest and src node IDs from packet
            if (strcmp(token, my_id) == 0) {
                // First token is own id, message is for this node
                token = strtok(NULL, " ");
                // Second token is sender id
                strncpy(sender_id, token, 6);

                // Check if third token is "ACK"
                token = strtok(NULL, " ");
                if (strcmp(token, "ACK") == 0) {
                    // Check if fourth token is "LFG-R"
                    token = strtok(NULL, " ");
                    if (strcmp(token, "LFG-R") == 0) {    
                        if (settings.debug) {
                            printf("Node %d received ACK\n", id);
                        }
                        mcu_return(nodes, id, own_function_number, 1);
                        return 0;
                    }
                }
            }
            // Not LFG-R ACK packet, keep listening
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        // check time
        if (nodes[id].tmp_start_time + 0.05 < state.current_time) {
            // time expired stop listening for replies, return to main
            nodes[id].tmp_start_time = FLT_MAX;
            mcu_return(nodes, id, own_function_number, 0);
            return 0;  
        }
        // Time not expired, continue
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            // Nothing heard, try again
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;            
        }
    }
    else {
        // Not returning from a call (first entry)
        // set start_time and check for activity on active channel
        nodes[id].tmp_start_time = state.current_time;
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;    
}

/**
 * Function Number:             14
 * Function Name:               group_cycle_start
 * Function Description:        initialize group cycle for node
 * Function Busy time:          0 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_group_cycle_start(struct Node* nodes, int id) {    
    int own_function_number = 14;

    // Reset timer
    nodes[id].group_cycle_start = state.current_cycle;

    // If broadcaster, clear group list
    if (nodes[id].broadcaster == 1) {
        for (int i = 0; i < settings.group_max; i++) {
            nodes[id].group_list[i] = -1;
        }
    }

    // Use broadcast_percentage to decide next role
    if (rand() % 100 < settings.broadcast_percentage) {
        nodes[id].broadcaster = 1;
    }
    else {
        nodes[id].broadcaster = 0;
    }
    
    mcu_return(nodes, id, own_function_number, 0);
    return 0;    
}

/**
 * Function Number:             15
 * Function Name:               sensor_data_send
 * Function Description:        Send sensor data to group broadcaster
 * Function Busy time:          0 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_sensor_data_send(struct Node* nodes, int id) {    
    int own_function_number = 15;

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
            // Send packet
            mcu_call(nodes, id, own_function_number, 1, 5);
            return 0;
        }
    }
    else if (nodes[id].return_stack->returning_from == 5) {
        // Returning from transmit_message_begin
        // No error checking for now
        if (settings.debug) {
            printf("Node %d sent \"%s\" on channel %d at tick %lu\n", id, 
                   nodes[id].send_packet, nodes[id].active_channel, state.current_cycle);
        }
        rs_pop(&nodes[id].return_stack);
        mcu_call(nodes, id, own_function_number, 2, 6);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 6) {
        //printf("Returned from function 6 at tick %lu\n", state.current_cycle);
        // Returning from transmit_message_complete
        rs_pop(&nodes[id].return_stack);
        mcu_return(nodes, id, own_function_number, 0);
        return 0;
    }
    else {
        // Update sensor data
        for (int i = 0; i < settings.sensor_count; i++) {
            update_sensor(nodes, id, i);
        }
    
        // Generate message to send
        char sensor_id[2];
        char message_time[10];

        snprintf(nodes[id].send_packet, sizeof(nodes[id].send_packet), "N-%d N-%d DATA ", 
                nodes[id].dest_node, id);
        for (int i = 0; i < settings.sensor_count; i++) {
            snprintf(sensor_id, 2, "%d", i);
            strncat(nodes[id].send_packet, "S", 2);
            strncat(nodes[id].send_packet, sensor_id, 3);
            strncat(nodes[id].send_packet, ": ", 3);
            strncat(nodes[id].send_packet, nodes[id].sensors[i].reading, READING_BUFFER_SIZE);
            strncat(nodes[id].send_packet, " ", 2);
        }
        snprintf(message_time, 10, "%f", state.current_time);
        strncat(nodes[id].send_packet, "TIME ", 6);
        strncat(nodes[id].send_packet, message_time, 11);
        
        // Check for activity on channel    
        mcu_call(nodes, id, own_function_number, 0, 4);
    }

    return 0;    
}

/**
 * Function Number:             16
 * Function Name:               sensor_data_recv
 * Function Description:        Receive sensor data from group nodes
 * Function Busy time:          0 
 * Function Return Labels:      0

 * Function Returns:            0 - void
**/
int mcu_function_sensor_data_recv(struct Node* nodes, int id) {    
    int own_function_number = 16;
    
    if (nodes[id].return_stack->returning_from == 7) {
        // Returning from receive function
        // Return value is sending node ID
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
        if (return_value == -1) {
            // collision detected try again 
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        if (return_value == -2) {
            // Nothing heard try again
            mcu_call(nodes, id, own_function_number, 1, 4);
            return 0;
        }
        else {
            // Check for DATA message
            char* token;
            char incoming_buffer[256];
            char message[256];

            // Zero out message string to eliminate proceeded garbage data
            bzero(message, 256);

            strncpy(incoming_buffer, nodes[return_value].send_packet, 256);

            token = strtok(incoming_buffer, " ");
            char my_id[6];
            char sender_id[6];
            snprintf(my_id, 6, "N-%d", id);
            
            // Extract dest and src node IDs from message
            if (strcmp(token, my_id) == 0) {
                token = strtok(NULL, " ");
                strncpy(sender_id, token, 6);

                // Check if third token is "DATA"
                token = strtok(NULL, " ");
                if (strcmp(token, "DATA") == 0) {
                    if (settings.debug) {
                        printf("Node %d heard DATA message from node %d at %lu\n", id, return_value, state.current_cycle);
                    }
                    // Process remaining tokens
                    token = strtok(NULL, " ");
                    do {
                        strncat(message, token, strlen(token));
                        strncat(message, " ", 2);
                        token = strtok(NULL, " ");
                    } while (token != 0);
                    if (settings.debug) {
                        printf("Message: %s\n", message);
                    }

                    // Add message to relay queue
                    //node[id].stored_messages = stored_message_create(node[id].stored_messages, sender_id, message);
                }
            }
        }
        // Keep listening
        mcu_call(nodes, id, own_function_number, 0, 4);
        return 0;
    }
    else if (nodes[id].return_stack->returning_from == 4) {
        // Returning from check_channel_busy function
        int return_value = nodes[id].return_stack->return_value;
        rs_pop(&nodes[id].return_stack);
    
        // Check cycle timer
        if (cycle_timer_check_expired(nodes[id].timers, own_function_number, 0)) {
            mcu_return(nodes, id, own_function_number, 0);
            return 0;
        }
        // time not expired, continue
        if (return_value == 1) {
            // Activity on channel, get packet
            mcu_call(nodes, id, own_function_number, 1, 7);
            return 0;
        }
        else {
            mcu_call(nodes, id, own_function_number, 0, 4);
            return 0;            
        }
    }
    else {
        // Not returning from a call (first entry)
        // set start_time and check for activity on active channel
        if (settings.debug) {
            printf("Node %d listening for DATA packets on channel %d\n", id, nodes[id].active_channel);
        }
        nodes[id].timers = 
            cycle_timer_create(nodes[id].timers, own_function_number, 0, state.current_cycle, 1000);
        mcu_call(nodes, id, own_function_number, 0, 4);
    }
    return 0;    
}