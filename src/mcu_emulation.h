/**
 * @file    mcu_emulation.h
 * @brief   Microcontroller emulation for dynamic wireless network simulation
 *
 * @author  Mitchell Clay
 * @date    12/26/2020
**/

#include "node.h"

#ifndef mcuemulation_H
#define mcuemulation_H

int update_mcu(struct Node*, int, double, int, int);
int mcu_run_function(struct Node*, int, int, double, int, int);
int mcu_function_scan_lfg(struct Node*, int, int, double, double, int, int);
int mcu_function_broadcast_lfg(struct Node*, int, double, double, int, int);
int mcu_function_find_clear_channel(struct Node*, int, int, double, double, int);

#endif