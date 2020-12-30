/**
 * @file    node.h
 * @brief   Node specific functions for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

#ifndef node_H
#define node_H

struct FS_Element{ 
    int data; 
    struct FS_Element* next; 
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
    int return_value;
    char* send_packet[256];
};

int initialize_nodes(struct Node*, int, double, double, double, double, double, double, int, char*, int, int); 
int update_acceleration(struct Node*, int, double, double, int);
int update_velocity(struct Node*, int, double, int);
int update_position(struct Node*, int, double, int);
int update_signal(struct Node*, int, int, int);
int write_node_data(struct Node*, int, int, double, FILE*);
void push(int, struct FS_Element**);
void pop(struct FS_Element**);

#endif