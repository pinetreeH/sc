//
// Created by pinetree on 16-9-10.
//

#ifndef SESSION_H
#define SESSION_H

//#define ROOM_NAME_MAX 32

#include "hashmap.h"
#include "minheap.h"

struct client;

extern int session_init(int capacity);

extern int add_new_client(int fd, const char *sid);

extern int delete_client_by_fd(int fd);

extern int delete_client_by_sid(const char *sid);

extern struct client *get_client_by_sid(const char *sid);

extern struct client *get_client_by_fd(int fd);

extern int get_min_time(void);

extern int is_new_connection(int fd, const char *data, int data_len);

extern int update_client_heartbeat_by_fd(int fd);


#endif
