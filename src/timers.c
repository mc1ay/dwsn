/**
 * @file    timers.c
 * @brief   Timer specific functions
 *
 * @author  Mitchell Clay
 * @date    2/19/2021
**/

#include <stdio.h>
#include "settings.h"
#include "state.h"

struct cycle_timer* cycle_timer_create(struct cycle_timer* head, int function, int label, unsigned long start) {
    struct cycle_timer* new_timer = malloc(sizeof(struct cycle_timer*));

    if (new_timer == NULL) {
        printf("Timer memory allocation error\n");
        exit(0);
    }
    new_timer->function = function;
    new_timer->label = label;
    new_timer->start = start;
    new_timer->next = head;

    return new_timer;
}

struct cycle_timer* cycle_timer_get(struct cycle_timer* head, int function, int label) {
 
    struct cycle_timer *cursor = head; 
    while (cursor != NULL) {
        if (cursor->function == function && cursor->label == label) {
            return cursor;
        }
        cursor = cursor->next;
    }
    return NULL;
}

struct cycle_timer* cycle_timer_remove(struct cycle_timer* head, struct cycle_timer* nd) {
    // first node
    if (nd == head) {
        struct cycle_timer* front = head;
        head = head->next;
        front->next = NULL;
        /* is this the last node in the list */
        if (front == head) {
            head = NULL;
        }
        free(front);
    }
 
    // last node 
    else if (nd->next == NULL) { 
        struct cycle_timer* cursor = head;
        struct cycle_timer* back = NULL;
        while (cursor->next != NULL) {
            back = cursor;
            cursor = cursor->next;
        }
        if (back != NULL) {
            back->next = NULL;
        }
    
        // if this is the last node in the list
        if (cursor == head) {
            head = NULL;
        }
    
        free(cursor);
    }
 
    // node is in the middle
    else {
        struct cycle_timer* cursor = head;
        while (cursor != NULL) {
            if (cursor->next == nd) {
                break;
            }
            cursor = cursor->next;
        }
    
        if (cursor != NULL) {
            struct cycle_timer* tmp = cursor->next;
            cursor->next = tmp->next;
            tmp->next = NULL;
            free(tmp);
        }
    }
    return head;
}