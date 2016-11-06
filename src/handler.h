//
// Created by pinetree on 16-9-19.
//
#ifndef SC_HANDLER_HDL_H
#define SC_HANDLER_HDL_H

struct handler_if;
struct client;
struct reactor_base;
struct session;
struct nsp;
struct server;

// send msg to the client that starts it.
extern int hdl_emit(struct client *c, struct reactor_base *ae,
                    const char *event, int event_len,
                    const char *msg, int len);

// send msg to all the clients which under the same nsp
extern int hdl_broadcast(struct session *s, struct reactor_base *ae,
                         struct client **except_clients, int client_size,
                         const char *event, int event_len,
                         const char *msg, int len);

// send msg to all the clients which under the specific nsp and room
extern int hdl_room_broadcast(struct nsp *n, struct reactor_base *ae,
                              const char *room_name,
                              struct client **except_clients, int client_size,
                              const char *event, int event_len,
                              const char *msg, int len);

extern int hdl_jion_room(struct nsp *n, const char *room_name, struct client *c);

extern int hdl_leave_room(struct nsp *n, const char *room_name, struct client *c);

extern int hdl_admin_recv_data(struct server *srv, int fd, const char *data, int len);

extern void hdl_server_accpet(struct reactor_base *base, int fd,
                              void *fn_parameter, int mask);

extern void hdl_admin_server_accpet(struct reactor_base *base, int fd,
                                    void *fd_parameter, int mask);

#endif
