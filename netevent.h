//
// Created by pinetree on 16-9-9.
//

#ifndef NET_H
#define NET_H

#include "reactor.h"

int init_socket(char *ip, int port);

void handle_server_accpet(struct reactor_base *base, int fd, void *fd_parameter,
                          int mask);


#endif
