//
// Created by pinetree on 16-9-19.
//
#ifndef SC_HANDLER_HDL_H
#define SC_HANDLER_HDL_H

struct handler_if;
struct client;

// send msg to the client that starts it.
extern int hdl_emit(struct client *c,
                    const char *event, int event_len,
                    const char *msg, int len);

// send msg to all the clients which under the same namespace
extern int hdl_broadcast(struct session *s,
                         struct client **except_clients, int client_size,
                         const char *event, int event_len,
                         const char *msg, int len);

// send msg to all the clients which under the specific namespace and room
extern int hdl_room_broadcast(const char *nsp_name, const char *room_name,
                              struct client **except_clients, int client_size,
                              const char *event, int event_len,
                              const char *msg, int len);

extern int hdl_admin_recv_data(int fd, const char *data, int len);

extern void hdl_server_accpet(struct reactor_base *base, int fd,
                              void *fn_parameter, int mask);

extern void hdl_admin_server_accpet(struct reactor_base *base, int fd,
                                    void *fd_parameter, int mask);

#endif
