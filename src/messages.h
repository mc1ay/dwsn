/**
 * @file    messages.h
 * @brief   Messages specific functions/structs
 *
 * @author  Mitchell Clay
 * @date    2/28/2021
**/

#ifndef messages_H
#define messages_H

#define STORED_MESSAGE_SIZE         256

struct stored_message {
    int sender;
    char message[STORED_MESSAGE_SIZE];
    struct stored_message* next;
};

struct stored_message* stored_message_create(struct stored_message* head, int sender, char* message);
struct stored_message* stored_message_remove(struct stored_message* head, struct stored_message* nd);   

#endif