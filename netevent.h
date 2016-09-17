//
// Created by pinetree on 16-9-9.
//

#ifndef NET_H
#define NET_H

#include "reactor.h"

void set_fd_nonblocking(int fd);

int init_socket(char *ip, int port);

void handle_server_accpet(struct reactor_base *base, int fd, void *fd_parameter,
                          int mask);

int tcp_send(int fd, const char *data, int len);

int tcp_recv(int fd, char *data, int len);


#endif
