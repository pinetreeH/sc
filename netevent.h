//
// Created by pinetree on 16-9-9.
//

#ifndef NET_H
#define NET_H

struct reactor_base;

extern int init_socket(char *ip, int port);

extern void handle_server_accpet(struct reactor_base *base, int fd,
                                 void *fd_parameter, int mask);


#endif
