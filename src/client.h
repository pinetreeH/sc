//
// Created by pinetree on 9/24/16.
//

#ifndef SC_CLIENT_CLENT_H
#define SC_CLIENT_CLENT_H

struct client;

extern struct client *client_new(void);

extern int client_del(void *c);

extern int client_set_sid(struct client *c, const char *sid);

extern int client_set_ip(struct client *c, const char *ip);

extern int client_set_port(struct client *c, int port);

extern int client_set_fd(struct client *c, int fd);

extern int client_set_heartbeat(struct client *c, int heartbeart);

extern const char *client_get_sid(struct client *c);

extern const char *client_get_ip(struct client *c);

extern int client_get_port(struct client *c);

extern int client_get_fd(struct client *c);

extern int client_get_heartbeat(struct client *c);

extern int client_set_hearbeat_ptr(struct client *c, void *ptr);

extern void *client_get_hearbeat_ptr(struct client *c);

#endif
