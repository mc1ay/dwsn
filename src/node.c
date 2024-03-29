/**
 * @file    node.c
 * @brief   Node specific functions for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "node.h"
#include "mcu_emulation.h"
#include "settings.h"
#include "state.h"
#include "timers.h"
#include <string.h>

extern struct Settings settings;
extern struct State state;

int initialize_nodes(struct Node* nodes) {
    char file_path[100];

    if (settings.debug) {
        printf("Setting inital node coordinates to %f %f %f\n", settings.start_x,
                settings.start_y, settings.start_z);
    }

    for (int i = 0; i < settings.node_count; i++) {
        nodes[i].terminal_velocity = 
            settings.terminal_velocity + 
           (settings.terminal_velocity * DRAGVARIANCE * (rand() % 201 - 100.0) / 100);
        nodes[i].x_pos = settings.start_x;
        nodes[i].y_pos = settings.start_y;
        nodes[i].z_pos = settings.start_z;
        nodes[i].x_velocity = 0;
        nodes[i].y_velocity = 0;
        nodes[i].z_velocity = 0;
        nodes[i].x_acceleration = 0;
        nodes[i].y_acceleration = 0;
        nodes[i].z_acceleration = settings.gravity;
        nodes[i].power_output = settings.default_power_output;
        nodes[i].transmit_active = 0;
        nodes[i].active_channel = 0;
        nodes[i].current_function = 0;
        nodes[i].busy_remaining = -1;
        nodes[i].received_signals = malloc(sizeof(double) * settings.node_count);
        nodes[i].group_list = malloc(sizeof(int) * settings.group_max);
        nodes[i].function_stack = malloc(sizeof(struct FS_Element));
        nodes[i].return_stack = malloc(sizeof(struct RS_Element));
        nodes[i].tmp_lfg_chans = malloc(sizeof(int) * settings.channels);
        nodes[i].tmp_scanned_chans = malloc(sizeof(int) * settings.channels);
        nodes[i].tmp_start_time = FLT_MAX;
        nodes[i].broadcaster = 0;
        nodes[i].group_cycle_start = 0;
        nodes[i].timers = malloc(sizeof(struct cycle_timer));
        nodes[i].sensors = malloc(sizeof(struct sensor) * settings.sensor_count);
        nodes[i].stored_messages = malloc(sizeof(struct stored_message));


        // Set all received signals to 0 initially
        for (int j = 0; j < settings.node_count; j++) {
            nodes[i].received_signals[j] = 0;
        }
        
        // Set up array for group members, use -1 for no node
        for (int j = 0; j < settings.group_max; j++) {
            nodes[i].group_list[j] = -1;
        }

        // Set sensor types
        for (int j = 0; j < settings.sensor_count; j++) {
            nodes[i].sensors[j].type = settings.sensor_types[j];
        }

        // Initialize timer head node
        nodes[i].timers->function = -1;
        nodes[i].timers->label = -1;
        nodes[i].timers->next = NULL;

        // Initialize stored message head node
        nodes[i].stored_messages->sender = -1;
        nodes[i].stored_messages->next = NULL;

        if (settings.output) {
            sprintf(file_path, "%s/node-%d%s", settings.output_dir, i, ".txt");
            if (settings.debug) {
                printf("Creating output file \"%s\"\n", file_path);
            }
            FILE *fp;
            fp  = fopen (file_path, "w");
            write_node_data(nodes, i, fp);
            fclose(fp);
        }
    }
    return 0;
}

int update_acceleration(struct Node* nodes) {
    for (int i = 0; i < settings.node_count; i++) {
        // update x/y acceleration
        // use spread_factor as percentage likelyhood that there is some change to acceleration
        if (rand() % 100 < settings.spread_factor) {
            // change x and y by random percentage of max allowed change per second
            double x_accel_change = (rand() % 201 - 100) / 100.0 
                                    * settings.time_resolution * XYACCELDELTAMAX;
            double y_accel_change = (rand() % 201 - 100) / 100.0 
                                    * settings.time_resolution * XYACCELDELTAMAX;
            if (settings.debug >= 3) {
                printf("Changing x/y accel for node %d by %f,%f\n", i, x_accel_change, y_accel_change);
            }
            nodes[i].x_acceleration += x_accel_change;
            nodes[i].y_acceleration += y_accel_change;
        }
        // update z acceleration 
        // for our purposes z always equals gravity so not update needed (just a placeholder)
    }
    return 0;
}

int update_velocity(struct Node* nodes) {
    for (int i = 0; i < settings.node_count; i++) {
        // update z velocity
        if (nodes[i].z_pos > 0) { 
            if (nodes[i].z_velocity < nodes[i].terminal_velocity) {
                if (nodes[i].z_velocity + (nodes[i].z_acceleration * 
                    settings.time_resolution) < nodes[i].terminal_velocity) {
                    nodes[i].z_velocity += (nodes[i].z_acceleration *
                                           settings.time_resolution);
                }
                else {
                    nodes[i].z_velocity = nodes[i].terminal_velocity;
                    if (settings.debug >=2) {
                        printf("Node %d reached terminal velocity of %f m/s\n", i, nodes[i].terminal_velocity);
                    }
                }
            }
        }
        // update x/y velocity
        nodes[i].x_velocity += (nodes[i].x_acceleration * settings.time_resolution);
        nodes[i].y_velocity += (nodes[i].y_acceleration * settings.time_resolution);
    }     
    return 0;
}

int update_position(struct Node* nodes) {
    for (int i = 0; i < settings.node_count; i++) {
        // Update z position
        if (nodes[i].z_pos > 0) { 
            if (nodes[i].z_pos - (nodes[i].z_velocity * settings.time_resolution) > 0) { 
                nodes[i].z_pos -= (nodes[i].z_velocity * settings.time_resolution);
            }
            else {
                nodes[i].z_pos = 0;
            }
        }
        // Update x/y position
        nodes[i].x_pos += (nodes[i].x_velocity * settings.time_resolution);
        nodes[i].y_pos += (nodes[i].y_velocity * settings.time_resolution);

    }
    return 0;
}

int update_signal(struct Node* nodes, int id, int target) {
    // Not taking noise floor into account currently
    // Check distance to other target node and calculate free space loss
    // to get received signal 
    double distance = sqrt(
        pow((nodes[id].x_pos - nodes[target].x_pos),2) +
        pow((nodes[id].y_pos - nodes[target].y_pos),2) +
        pow((nodes[id].z_pos - nodes[target].z_pos),2) 
    );
    nodes[id].received_signals[target] = nodes[target].power_output -
        (20 * log(distance) + 20 * log(2400) + 32.44);
    return 0;
}

int write_node_data(struct Node* nodes, int id, FILE *fp) {
    char buffer[100 + settings.node_count * 15];
    sprintf(buffer, "%f\t%i\t%i\t%f\t%f\t%f ", state.current_time, 
                                          nodes[id].active_channel,
                                          nodes[id].current_function, 
                                          nodes[id].x_pos, 
                                          nodes[id].y_pos, 
                                          nodes[id].z_pos);
    fputs(buffer, fp);
    for (int i = 0; i < settings.node_count; i++) {
        if (i < settings.node_count - 1) {
            sprintf(buffer, "%f\t", nodes[id].received_signals[i]);
        }
        else {
            sprintf(buffer, "%f", nodes[id].received_signals[i]);
        }
        fputs(buffer, fp);
    }
    sprintf(buffer,"\n");
    fputs(buffer, fp);

    return 0;
}

void fs_push(int caller, int return_to_label, struct FS_Element** stack){
    struct FS_Element* element = (struct FS_Element*)malloc(sizeof(struct FS_Element)); 
    element -> caller = caller; 
    element -> return_to_label = return_to_label;
    element -> next = *stack;  
    (*stack) = element;  
}

void fs_pop(struct FS_Element** stack){
    if(*stack != NULL){
        struct FS_Element* tempPtr = *stack;
        *stack = (*stack) -> next;
        free(tempPtr);
    }
    else{
        printf("The stack is empty.\n");
    }
}

void rs_push(int returning_from, int return_to_label, int return_value, struct RS_Element** stack){
    struct RS_Element* element = (struct RS_Element*)malloc(sizeof(struct RS_Element)); 
    element -> returning_from = returning_from; 
    element -> return_to_label = return_to_label;
    element -> return_value = return_value;
    element -> next = *stack;  
    (*stack) = element;  
}

void rs_pop(struct RS_Element** stack){
    if(*stack != NULL){
        struct RS_Element* tempPtr = *stack;
        *stack = (*stack) -> next;
        free(tempPtr);
    }
    else{
        printf("The stack is empty.\n");
    }
}

int update_sensor(struct Node* nodes, int id, int sensor_number) {
    // Update sensor based on sensor type
    if (nodes[id].sensors[sensor_number].type == SENSOR_TYPE_TEMP) {
        // not yet implemented, use generic value for now
        snprintf(nodes[id].sensors[sensor_number].reading, READING_BUFFER_SIZE, "%f", 20.0);
    }
    else if (nodes[id].sensors[sensor_number].type == SENSOR_TYPE_ACCELEROMETER) { 
        snprintf(nodes[id].sensors[sensor_number].reading, READING_BUFFER_SIZE, "%f %f %f",
                 nodes[id].x_acceleration,
                 nodes[id].y_acceleration,
                 nodes[id].z_acceleration);
    }
    else if (nodes[id].sensors[sensor_number].type == SENSOR_TYPE_ALTIMETER) {
        snprintf(nodes[id].sensors[sensor_number].reading, READING_BUFFER_SIZE, "%f", nodes[id].z_pos);
    }
    else if (nodes[id].sensors[sensor_number].type == SENSOR_TYPE_GPS) { 
        snprintf(nodes[id].sensors[sensor_number].reading, READING_BUFFER_SIZE, "%f %f %f",
                 nodes[id].x_pos,
                 nodes[id].y_pos,
                 nodes[id].z_pos);
    }
    return 0;
}