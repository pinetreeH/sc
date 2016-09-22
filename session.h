//
// Created by pinetree on 16-9-10.
//

#ifndef SESSION_H
#define SESSION_H

#define CLIENT_SID_MAX 32
//#define ROOM_NAME_MAX 32

#include "hashmap.h"
#include "minheap.h"

struct client {
    char sid[CLIENT_SID_MAX];
    int fd;
    int heartbeat;
    heap_element *heartbeat_in_sesssion;
};

struct fd_client {
    int fd;
    struct client *client;
};

struct session {
    int capacity;
    int size;
    struct fd_client *fd_to_client;// array
    hashmap *sid_to_client; // sid is a string
    heap *heartbeat;
};


int session_init(int capacity);

int add_new_client(int fd, const char *sid);

int delete_client_by_fd(int fd);

int delete_client_by_sid(const char *sid);

struct client *get_client_by_sid(const char *sid);

struct client *get_client_by_fd(int fd);

int get_min_time(void);

int is_new_connection(int fd, const char *data, int data_len);

int update_client_heartbeat_by_fd(int fd);


#endif
