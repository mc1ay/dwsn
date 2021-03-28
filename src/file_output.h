/**
 * @file    file_output.h
 * @brief   File output functions 
 *
 * @author  Mitchell Clay
 * @date    1/3/2021
**/

#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "node.h"
#include "settings.h"

#ifndef fileoutput_H
#define fileoutput_H

int check_write_interval(struct Node*);
int create_log_dir();
int create_transmit_history_file();
int create_ground_received_file();
int log_ground_received_message(char*, int);

#endif