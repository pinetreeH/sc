//
// Created by pinetree on 16-9-10.
//

#ifndef SESSION_H
#define SESSION_H

#define CLIENT_SID_MAX 32
//#define ROOM_NAME_MAX 32

#include "hashmap.h"

struct client {
    char sid[CLIENT_SID_MAX];
    int fd;
    int heartbeat;
};

struct fd_client {
    int fd;
    struct client *client;
};

struct session {
    int capacity;
    int size;

    struct fd_client *fd_to_client;
    hashmap *sid_to_client; // sid is a string
};


int session_init(int capacity);

int add_new_client(int fd, const char *sid);

int delete_client_by_fd(int fd);

int delete_client_by_sid(const char *sid);

struct client *get_client_by_sid(char *sid);

struct client *get_client_by_fd(int fd);

int get_min_time(void);


#endif
