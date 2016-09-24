//
// Created by pinetree on 9/24/16.
//

#ifndef SC_CLIENT_H
#define SC_CLIENT_H

#include "minheap.h"
#include <arpa/inet.h>

#define CLIENT_SID_MAX 48

struct client {
    // TODO ipv6
    char ip[INET_ADDRSTRLEN];
    int port;
    char sid[CLIENT_SID_MAX];
    int fd;
    int heartbeat;
    heap *heartbeat_in_ses;
};

#endif //SC_CLIENT_H
