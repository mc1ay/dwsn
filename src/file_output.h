/**
 * @file    file_output.h
 * @brief   File output functions 
 *
 * @author  Mitchell Clay
 * @date    1/3/2021
**/

#include "node.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#ifndef fileoutput_H
#define fileoutput_H

int check_write_interval(struct Node*, int, int, double*, double, double, char*, int);
int create_log_dir(char*, int);
int create_transmit_history_file(char*, int);

#endif