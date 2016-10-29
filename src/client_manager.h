//
// Created by pinetree on 16-10-29.
//

#ifndef SC_CLIENT_MANAGER_CMG_H
#define SC_CLIENT_MANAGER_CMG_H

struct reactor_base;
struct client;
struct client_manager;

extern struct client_manager *cmg_init(int capacity);

extern int cmg_add_client(struct client_manager *cmg, struct client *c);

extern int cmg_del_client_by_fd(struct client_manager *cmg, int fd);

extern struct client *cmg_get_client_by_fd(struct client_manager *cmg, int fd);

extern struct client *cmg_get_client_by_sid(struct client_manager *cmg,
                                            const char *sid);

extern int cmg_update_client_heartbeat_by_fd(struct client_manager *cmg,
                                             int fd, int new_heartbeat);

extern int cmg_handle_timeout_client(struct client_manager *cmg,
                                     struct reactor_base *ae,
                                     void *heartbeat_timeout);

#endif
