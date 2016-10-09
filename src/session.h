//
// Created by pinetree on 16-9-10.
//

#ifndef SC_SESSION_SES_H
#define SC_SESSION_SES_H

//#define ROOM_NAME_MAX 32

#include "hashmap.h"

struct client;

struct reactor_base;

extern int ses_init(int capacity);

extern int ses_add_new_client(int fd, const char *sid);

extern int ses_del_client_by_fd(int fd);

extern int ses_del_client_by_sid(const char *sid);

extern int ses_new_connection(int fd, const char *data, int data_len);

extern int ses_update_client_heartbeat_by_fd(int fd);

extern struct client **ses_get_clients(int *size);

extern struct client *ses_get_client_by_fd(int fd);

extern struct client *ses_get_client_by_sid(const char *sid);

extern int ses_handle_timeout_client(struct reactor_base *ae, void *heartbeat_timeout);

extern int ses_room_join(const char *room_name, struct client *c);

extern int ses_room_leave(const char *room_name, struct client *c);

extern hashmap *ses_get_room(const char *room_name);

#endif
