/**
 * @file    mcu_functions.h
 * @brief   Microcontroller functions 
 *
 * @author  Mitchell Clay
 * @date    1/1/2021
**/

#include <string.h>
#include "node.h"
#include "mcu_emulation.h"

#ifndef mcufunctions_H
#define mcufunctions_H

int mcu_function_main(struct Node*, int, int, int, int, int);
int mcu_function_scan_lfg(struct Node*, int, int, int, int, int);
int mcu_function_broadcast_lfg(struct Node*, int, int, int);
int mcu_function_find_clear_channel(struct Node*, int, int, int);
int mcu_function_check_channel_busy(struct Node*, int, int, int);
int mcu_function_transmit_message_begin(struct Node*, int, int, int);
int mcu_function_transmit_message_complete(struct Node*, int, int, int);
int mcu_function_receive(struct Node*, int, int, int);
int mcu_function_sleep(struct Node*, int, int, int);
int mcu_function_respond_lfg(struct Node*, int, int, int);
int mcu_function_scan_lfg_responses(struct Node*, int, int, double*, int, int);


#endif