//
// Created by pinetree on 9/24/16.
//

#ifndef SC_CLIENT_H
#define SC_CLIENT_H

#include "heap.h"
#include <arpa/inet.h>

#define CLIENT_SID_MAX 48
#define CLIENT_NSP_MAX 32
#define CLIENT_BUF_MAX (1024*16)

struct client {
    // default transport "websocket"
    // TODO ipv6
    char sid[CLIENT_SID_MAX];
    char ip[INET_ADDRSTRLEN];
    int port;
    int fd;
    int heartbeat;
    heap *heartbeat_in_ses;
    char nsp[CLIENT_NSP_MAX];
    //
    char read_buf[CLIENT_BUF_MAX];
    char write_buf[CLIENT_BUF_MAX];
    int read_start_idx;
    int read_end_idx;
    int write_stat_idx;
    int write_end_idx;
};

#endif //SC_CLIENT_H
