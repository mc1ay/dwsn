/**
 * @file    messages.c
 * @brief   Messages specific functions
 *
 * @author  Mitchell Clay
 * @date    2/28/2021
**/

#include <stdio.h>
#include <string.h>
#include "messages.h"
#include "settings.h"
#include "state.h"

extern struct State state;

struct stored_message* stored_message_create(struct stored_message* head, 
                                             int sender, 
                                             char* message) {
    // Allocate memory for new message node
    struct stored_message* new_message = malloc(sizeof(struct stored_message));
    if (new_message == NULL) {
        printf("message memory allocation error\n");
        exit(0);
    }

    // Assign parameters to new message node
    new_message->sender = sender;
    new_message->next = head;
    strncpy(new_message->message, message, STORED_MESSAGE_SIZE);

    return new_message;
}

struct stored_message* stored_message_remove(struct stored_message* head, struct stored_message* nd) {
    struct stored_message* front = head;
    head = head->next;
    front->next = NULL;
    free(front);

    return head;
}