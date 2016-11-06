//
// Created by pinetree on 9/24/16.
//

#ifndef SC_CLIENT_CLENT_H
#define SC_CLIENT_CLENT_H

struct client;
struct reactor_base;

extern struct client *client_new(int fd, const char *sid);

extern int client_del(void *c);

extern int client_set_heartbeat(struct client *c, int heartbeart);

extern const char *client_sid(struct client *c);

extern const char *client_ip(struct client *c);

extern int client_port(struct client *c);

extern int client_fd(struct client *c);

extern int client_set_room(struct client *c, const char *room_name);

extern const char *client_room_name(struct client *c);

extern int client_heartbeat(struct client *c);

extern int client_set_hearbeat_ptr(struct client *c, void *ptr);

extern void *client_get_hearbeat_ptr(struct client *c);

extern int client_send_data(struct client *c, const char *data, int len,
                            struct reactor_base *ae);

#endif
