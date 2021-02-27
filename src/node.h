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
#include "timers.h"

#ifndef node_H
#define node_H

#define XYACCELDELTAMAX 0.005
#define DRAGVARIANCE 0.05

#define SENSOR_TYPE_TEMP            0
#define SENSOR_TYPE_ACCELEROMETER   1
#define SENSOR_TYPE_ALTIMETER       2
#define SENSOR_TYPE_GPS             3

#define READING_BUFFER_SIZE         64

struct sensor {
    int type;
    char reading[READING_BUFFER_SIZE];
};

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
    int* tmp_scanned_chans;
    double tmp_start_time;
    int dest_node;
    int broadcaster;
    unsigned long group_cycle_start;
    struct cycle_timer* timers;
    struct sensor* sensors;
};

int initialize_nodes(struct Node*); 
int update_acceleration(struct Node*);
int update_velocity(struct Node*);
int update_position(struct Node*);
int update_signal(struct Node*, int, int);
int write_node_data(struct Node*, int, FILE*);
void fs_push(int, int, struct FS_Element**);
void fs_pop(struct FS_Element**);
void rs_push(int, int, int, struct RS_Element**);
void rs_pop(struct RS_Element**);
int update_sensor(struct Node*, int, int);

#endif