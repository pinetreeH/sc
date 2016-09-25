//
// Created by pinetree on 9/24/16.
//

#ifndef SC_CLIENT_H
#define SC_CLIENT_H

#include "minheap.h"
#include <arpa/inet.h>

#define CLIENT_SID_MAX 48
#define CLIENT_NSP_MAX 32

struct client {
    // TODO ipv6
    char sid[CLIENT_SID_MAX];
    char ip[INET_ADDRSTRLEN];
    char nsp[CLIENT_NSP_MAX];
    int port;
    int fd;
    int heartbeat;
    heap *heartbeat_in_ses;
};

#endif //SC_CLIENT_H
