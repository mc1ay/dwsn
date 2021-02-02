/**
 * @file    node.h
 * @brief   Node specific functions for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "settings.h"

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

#ifndef node_H
#define node_H

// Function stack element for mcu function emulation
struct FS_Element{ 
    int caller;
    int return_to_label;                                           
    struct FS_Element* next; 
};

// Function stack element for mcu function emulation
struct RS_Element{
    int returning_from;
    int return_to_label;
    int return_value;
    struct RS_Element* next;
};

struct Node {
    double terminal_velocity;
    double x_pos;
    double y_pos;
    double z_pos;
    double x_velocity;
    double y_velocity;
    double z_velocity;
    double x_acceleration;
    double y_acceleration;
    double z_acceleration;
    double power_output;
    int transmit_active;
    int active_channel;
    int current_function;
    double busy_remaining;
    double* received_signals;
    int* group_list;
    struct FS_Element* function_stack;
    struct RS_Element* return_stack;
    char send_packet[256];
    int* tmp_lfg_chans;
    double tmp_start_time;
    int dest_node;
};

int initialize_nodes(struct Node*, int, double, int, int, int, int); 
int update_acceleration(struct Node*, int, double, int);
int update_velocity(struct Node*, int, double, int);
int update_position(struct Node*, int, double, int);
int update_signal(struct Node*, int, int, int);
int write_node_data(struct Node*, int, int, double, FILE*);
void fs_push(int, int, struct FS_Element**);
void fs_pop(struct FS_Element**);
void rs_push(int, int, int, struct RS_Element**);
void rs_pop(struct RS_Element**);

#endif