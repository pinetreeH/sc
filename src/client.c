//
// Created by pinetree on 16-10-29.
//

#include "client.h"
#include "heap.h"
#include "util.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define CLIENT_SID_MAX 48
#define CLIENT_NSP_MAX 32
#define CLIENT_BUF_MAX (1024*16)

struct client {
    char sid[CLIENT_SID_MAX];
    char ip[INET_ADDRSTRLEN];
    int port;
    int fd;
    int heartbeat;
    void *heartbeat_ptr;
    char read_buf[CLIENT_BUF_MAX];
    char write_buf[CLIENT_BUF_MAX];
    int read_start_idx;
    int read_end_idx;
    int write_stat_idx;
    int write_end_idx;
};

struct client *client_new(void) {
    return (struct client *) mem_malloc(sizeof(struct client));
}

int client_del(void *c) {
    mem_free((void *) c);
}

int client_set_sid(struct client *c, const char *sid) {
    if (c && sid) {
        strcpy(c->sid, sid);
        return 0;
    }
    return -1;
}

int client_set_ip(struct client *c, const char *ip) {
    if (c && ip) {
        strcpy(c->ip, ip);
        return 0;
    }
    return -1;
}

int client_set_port(struct client *c, int port) {
    if (c && port) {
        c->port = port;
        return 0;
    }
    return -1;
}

int client_set_fd(struct client *c, int fd) {
    if (c && fd) {
        c->fd = fd;
        return 0;
    }
    return -1;
}

int client_set_heartbeat(struct client *c, int heartbeart) {
    if (c && heartbeart) {
        c->heartbeat = heartbeart;
        return 0;
    }
    return -1;
}

const char *client_get_sid(struct client *c) {
    return c ? c->sid : NULL;
}

const char *client_get_ip(struct client *c) {
    return c ? c->ip : NULL;
}

int client_get_port(struct client *c) {
    return c ? c->port : -1;
}

int client_get_fd(struct client *c) {
    return c ? c->fd : -1;
}

int client_get_heartbeat(struct client *c) {
    return c ? c->heartbeat : -1;
}

int client_set_hearbeat_ptr(struct client *c, void *ptr) {
    if (c && ptr)
        c->heartbeat_ptr = ptr;
    return 0;
}

void *client_get_hearbeat_ptr(struct client *c) {
    return c ? c->heartbeat_ptr : NULL;
}