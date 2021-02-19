/**
 * @file    timers.h
 * @brief   Timer specific functions/structures 
 *
 * @author  Mitchell Clay
 * @date    2/19/2021
**/

#ifndef timers_H
#define timers_H

struct cycle_timer {
    int function;
    int label;
    unsigned long start;
    struct cycle_timer* next;
};

struct cycle_timer* cycle_timer_create(struct cycle_timer* head, int function, int label, unsigned long start);
struct cycle_timer* cycle_timer_get(struct cycle_timer* head, int function, int label);
struct cycle_timer* cycle_timer_remove(struct cycle_timer* head, struct cycle_timer* nd);

#endif