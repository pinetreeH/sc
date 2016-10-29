//
// Created by pinetree on 16-9-19.
//
#ifndef SC_HANDLER_HDL_H
#define SC_HANDLER_HDL_H

struct handler_if;

struct client;

extern int hdl_recv_data(int fd, const char *data, int len);

extern int hdl_recv_close(int fd);

extern int hdl_recv_err(int fd);

extern int hdl_register_handler(const char *nsp_name, struct handler_if *h);

// send msg to the client that starts it.
extern int hdl_emit(const char *nsp_name, struct client *c,
                    const char *event, int event_len,
                    const char *msg, int len);

// send msg to all the clients which under the same nsp
extern int hdl_broadcast(const char *nsp_name,
                         struct client **except_clients, int client_size,
                         const char *event, int event_len,
                         const char *msg, int len);

// send msg to all the clients which under the specific nsp and room
extern int hdl_room_broadcast(const char *nsp_name, const char *room_name,
                              struct client **except_clients, int client_size,
                              const char *event, int event_len,
                              const char *msg, int len);


extern int hdl_admin_recv_data(int fd, const char *data, int len);

extern int hdl_init(void);

#endif
