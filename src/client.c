//
// Created by pinetree on 16-10-29.
//

#include "client.h"
#include "room.h"
#include "reactor.h"
#include "util.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define CLIENT_SID_MAX 48
#define CLIENT_BUF_MAX (1024*16)

struct client {
    char sid[CLIENT_SID_MAX];
    char ip[INET_ADDRSTRLEN];
    int fd;
    int heartbeat;
    void *heartbeat_ptr;
    char room_name[ROOM_NAME_STR_MAX]; // only in one room one time
    char send_buf[CLIENT_BUF_MAX];
    char recv_buf[CLIENT_BUF_MAX];
    short int port;
    short int send_end_idx;
    short int send_start_idx;
    short int recv_start_idx;
    short int recv_end_idx;
};

struct client *client_new(int fd, const char *sid) {
    struct client *c = NULL;
    c = (struct client *) mem_calloc(1, sizeof(struct client));
    if (c) {
        c->fd = fd;
        strcpy(c->sid, sid);
        c->heartbeat = util_get_timestamp();
        util_get_fd_ip_port(fd, c->ip, &c->port);
    }
    return c;
}

int client_del(void *c) {
    mem_free(c);
}

int client_set_heartbeat(struct client *c, int heartbeart) {
    if (c && heartbeart) {
        c->heartbeat = heartbeart;
        return 0;
    }
    return -1;
}

const char *client_sid(struct client *c) {
    return c ? c->sid : NULL;
}

const char *client_ip(struct client *c) {
    return c ? c->ip : NULL;
}

int client_port(struct client *c) {
    return c ? c->port : -1;
}

int client_fd(struct client *c) {
    return c ? c->fd : -1;
}

const char *client_room_name(struct client *c) {
    return c ? c->room_name : NULL;
}

int client_set_room(struct client *c, const char *room_name) {
    if (!c)
        return -1;
    if (room_name)
        strcpy(c->room_name, room_name);
    else
        memset(c->room_name, 0, ROOM_NAME_STR_MAX);
    return 0;
}

int client_heartbeat(struct client *c) {
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

static client_send_buf_data(struct reactor_base *ae, int fd,
                            void *fn_parameter, int mask) {
    UTIL_NOTUSED(fd);
    UTIL_NOTUSED(mask);

    struct client *c = (struct client *) fn_parameter;
    if (c->send_end_idx - c->send_start_idx <= 0) {
        // error
        log_err("client_send_buf_data,send_end_idx(%d)<=send_start_idx(%d) !"
                        "client sid:%s",
                c->send_end_idx, c->send_start_idx, c->sid);
        return -1;
    }

    int sent_size = 0;
    sent_size = util_tcp_send(c->fd, c->send_buf + c->send_start_idx,
                              c->send_end_idx - c->send_start_idx);
    if (sent_size == c->send_end_idx - c->send_start_idx) {
        c->send_start_idx = c->send_end_idx = 0;
        ae_del_net_event(ae, c->fd, AE_NET_EVENT_WRITE);
    } else if (sent_size == -1) {
        // error
        log_err("client_send_buf_data, sent_size == -1!, "
                        "client sid:%s,data:%s,len:%d",
                c->sid, c->send_buf, c->send_end_idx);
    } else {
        c->send_start_idx += sent_size;
    }

    return 0;
}

int client_send_data(struct client *c, const char *data, int len,
                     struct reactor_base *ae) {
    // check if client's send buf is empty
    if (c->send_end_idx - c->send_start_idx > 0) {
        // if not empty, just drop this data(resize socket buf size or ...)
        log_err("client_send_data, drop data,sidx:%d,eidx:%d "
                        "client sid:%s,data:%s, data_len:%d",
                c->send_start_idx, c->send_end_idx, c->sid, data, len);
        return -1;
    }

    int send_size = 0;
    send_size = util_tcp_send(c->fd, data, len);
    if (send_size == len) {
        return 0;
    } else if (send_size == -1) {
        //handle error
        log_err("client_send_data, send_size == -1!, "
                        "client sid:%s,data:%s,len:%d",
                c->sid, c->send_buf, c->send_end_idx);
        return -1;
    } else {
        // copy to send buf
        c->send_start_idx = 0;
        c->send_end_idx = len - send_size;
        memcpy(c->send_buf, data + send_size, c->send_end_idx);
        ae_add_net_event(ae, c->fd, AE_NET_EVENT_WRITE,
                         client_send_buf_data,
                         (void *) c, "client_send_data");
    }

    return 0;
}