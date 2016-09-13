//
// Created by pinetree on 16-9-13.
//

#ifndef CLIENT_H
#define CLIENT_H

#include "room.h"

#define CLIENT_SID_MAX 32
#define CLIENT_ROOM_MAX 32

struct client_info {
    int sockfd;
    char sid[CLIENT_SID_MAX];
    struct room *rooms;
    char *transport_name;
    void *write_buf;
    void *read_buf;
    int write_buf_len;
    int read_buf_len;
    int heartbeat;
};


#endif
