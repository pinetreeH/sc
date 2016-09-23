//
// Created by pinetree on 16-9-9.
//

#ifndef SC_NETEVENT_NET_H
#define SC_NETEVENT_NET_H

struct reactor_base;

extern int net_init_socket(char *ip, int port);

extern void net_server_accpet(struct reactor_base *base, int fd,
                              void *fd_parameter, int mask);


#endif
