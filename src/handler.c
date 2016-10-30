//
// Created by pinetree on 16-9-19.
//

#include "handler.h"
#include "handler_if.h"
#include "session.h"
#include "transport.h"
#include "reactor.h"
#include "hashmap.h"
#include "client.h"
#include "util.h"
#include "helper.h"
#include "server.h"
#include <string.h>
#include <stdio.h>

#define TMP_MSG_MAX 256
#define TCP_MSG_BUF_LEN (1024*16)

static int handle_new_connection(struct session *s, int fd,
                                 const char *data, int len) {
    // this is a new client, parse http request
    struct http_request_info info;
    tra_parse_http_req(data, len, &info);
    // new client has no sid and transport should be "websocket"
    if (!info.has_sid && tra_valid_transport(info.transport)) {
        char websocket_res[TRA_WS_RESP_MAX] = {'\0'};
        // TODO
        tra_ws_resp(data, websocket_res, TRA_WS_RESP_MAX);
        util_tcp_send(fd, websocket_res, strlen(websocket_res));
        // send {sid,upgrade,pingTimeout} ...
        char sid[TRA_SID_STR_MAX] = {'\0'};
        util_gen_sid_by_fd(fd, sid);
        char transport_config_msg[TMP_MSG_MAX];
        tra_get_conf(sid, transport_config_msg);
        int config_msg_len = strlen(transport_config_msg);
        char encode_msg[TMP_MSG_MAX];
        int encode_data_len = tra_eio_encode(TRA_EIO_PACKET_OPEN,
                                             transport_config_msg,
                                             config_msg_len, encode_msg,
                                             TMP_MSG_MAX);

        util_tcp_send(fd, encode_msg, encode_data_len);
        // after send config msg, we create a new client
        struct client *c = client_new(fd, sid);
        ses_add_client(s, c);
        // send CONNECT packet
        client_send_data(c, tra_get_sio_connect_packet(),
                         tra_get_sio_connect_packet_len());
    }
    return 0;
}

static int handle_eio_ping_packet(struct session *s, int fd) {
    // update client heartbeat
    struct client *c = NULL;
    c = ses_get_client_by_fd(s, fd);
    ses_update_client_heartbeat(s, c, util_get_timestamp());
    // send PONG packet
    return client_send_data(c, tra_get_sio_pong_packet(),
                            tra_get_sio_pong_packet_len());
}

static int handle_sio_msg_packet(struct handler_if *handler, int fd,
                                 const char *data, int len) {
    log_debug("EIO_PACKET_MESSAGE content,%s\n", data);
    tra_sio_packet_type sio_type = tra_sio_decode(data, len);

    switch (sio_type) {
        case TRA_SIO_PACKET_CONNECT:
            handler->on_connect(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_DISCONNECT:
            handler->on_disconnect(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_EVENT:
            handler->on_event(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ACK:
            handler->on_ack(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ERROR:
            handler->on_error(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_EVENT:
            handler->on_binary_event(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_ACK:
            handler->on_binary_ack(fd, data + 1, len - 1);
            break;
        default:
            break;
    }
    return 0;
}

static int handle_client_msg(struct server *srv, int fd, const char *data, int len) {
    char msg[TMP_MSG_MAX] = {0};
    int msg_len = tra_ws_get_content(data, len, msg, TMP_MSG_MAX);

    tra_eio_packet_type etype = tra_eio_decode(msg, msg_len);
    switch (etype) {
        case TRA_EIO_PACKET_OPEN:
            break;
        case TRA_EIO_PACKET_CLOSE:
            break;
        case TRA_EIO_PACKET_PING:
            handle_eio_ping_packet(srv->ses, fd);
            break;
        case TRA_EIO_PACKET_PONG:
            // this case will not happen
            break;
        case TRA_EIO_PACKET_MESSAGE:
            handle_sio_msg_packet(srv->handler, fd, msg + 1, msg_len - 1);
            break;
        case TRA_EIO_PACKET_UPGRADE:
            // this case will not happen
            break;
        case TRA_EIO_PACKET_NOOP:
            // this case will not happen
            break;
        default:
            break;
    }
    return 0;
}

int hdl_admin_recv_data(int fd, const char *data, int len) {
    log_debug("hdl_admin_recv_data,fd:%d,data:%s\n", fd, data);
    char test_admin_msg[TMP_MSG_MAX] = {0};
    if (len > 0) {
        int idx = 0;
        for (; idx < len && idx < TMP_MSG_MAX - 1 && data[idx] != '\r'; idx++) {
            test_admin_msg[idx] = data[idx];
        }
        test_admin_msg[idx] = '\0';
        char *event = "\"news\"";
        char bro_msg[TMP_MSG_MAX * 2] = {0};
        sprintf(bro_msg, "{\"admin\":\"msg:%s\"}", test_admin_msg);
        log_debug("admin_msg:%s\n", bro_msg);
        //hdl_broadcast(NULL, 0, event, strlen(event), bro_msg, strlen(bro_msg));
    }
}

static int in_except_list(struct client **except_clients,
                          int client_size, struct client *c) {
    if (!except_clients || client_size == 0)
        return 0;

    for (int i = 0; i < client_size; i++) {
        if (except_clients[i] == c) {
            return 1;
        }
    }
    return 0;
}

int hdl_emit(struct client *c,
             const char *event, int event_len,
             const char *msg, int len) {
    if (!c || !event || !msg || len <= 0)
        return -1;
    char encode_data[TRA_WS_RESP_MAX] = {0};
    int encode_len = -1;
    encode_len = tra_sio_encode(TRA_SIO_PACKET_EVENT,
                                NULL, 0,
                                event, event_len, msg, len,
                                encode_data, TRA_WS_RESP_MAX);

    log_debug("hdl_emit data len:%d\n", encode_len);
    client_send_data(c, encode_data, encode_len);
    return 0;
}

int hdl_broadcast(struct session *s,
                  struct client **except_clients, int client_size,
                  const char *event, int event_len,
                  const char *msg, int len) {

    hashmap *rooms = NULL;
    rooms = ses_get_rooms(nsp_name);
    if (!rooms)
        return 0;
    if (hashmap_size(rooms) == 0)
        return 0;

    char *room_name = NULL;
    hashmap *one_room = NULL;
    hashmap_iterator it = hashmap_get_iterator(rooms);
    while (hashmap_valid_iterator(it)) {
        it = hashmap_next(it, (void **) &room_name, (void **) &one_room);
        hdl_room_broadcast(nsp_name, room_name, except_clients, client_size,
                           event, event_len, msg, len);
    }

    return 0;
}

int hdl_room_broadcast(const char *nsp_name, const char *room_name,
                       struct client **except_clients, int client_size,
                       const char *event, int event_len,
                       const char *msg, int len) {
    if (!nsp_name || !room_name || !event || !msg)
        return -1;

    hashmap *room = NULL;
    room = ses_get_room(nsp_name, room_name);
    if (!room)
        return 0;

    struct client *c = NULL;
    hashmap_iterator it = hashmap_get_iterator(room);
    while (hashmap_valid_iterator(it)) {
        it = hashmap_next(it, (void **) &c, NULL);
        if (c && !in_except_list(except_clients, client_size, c))
            hdl_emit(nsp_name, c, event, event_len, msg, len);
    }

    return 0;
}

static void handle_tcp_recv(struct reactor_base *base, int fd,
                            void *fn_parameter, int mask) {
    UTIL_NOTUSED(base);
    UTIL_NOTUSED(mask);
    struct server *srv = (struct server *) fn_parameter;

    char buf[TCP_MSG_BUF_LEN] = {'\0'};
    int buf_len = util_tcp_recv(fd, buf, TCP_MSG_BUF_LEN);
    if (buf_len == 0) {
        log_debug("handle_tcp_recv buf_len==0\n");
        // close fd and client
        ae_del_net_event(base, fd, AE_NET_EVENT_READ | AE_NET_EVENT_WRITE);
        ses_del_client_by_fd(srv->ses, fd);
        util_tcp_shutdown(fd, 2);
    } else if (buf_len < 0) {
        log_debug("handle_tcp_recv buf_len<0,%d\n", buf_len);
        ae_del_net_event(base, fd, AE_NET_EVENT_READ | AE_NET_EVENT_WRITE);
        ses_del_client_by_fd(srv->ses, fd);
        util_tcp_shutdown(fd, 2);
    } else {
        log_debug("handle_tcp_recv:%d,%s\n", buf_len, buf);
        if (ses_is_new_connection(srv->ses, fd, buf, buf_len)) {
            log_debug("ses_new_connection,fd:%d\n", fd);
            handle_new_connection(srv->ses, fd, buf, buf_len);
        } else {
            // exist client, parse msg by transport protocol
            log_debug("exist client,fd:%d\n", fd);
            handle_client_msg(srv, fd, buf, buf_len);
        }
    }
}

static void handle_admin_tcp_recv(struct reactor_base *base, int fd,
                                  void *fn_parameter, int mask) {
    UTIL_NOTUSED(base);
    UTIL_NOTUSED(fn_parameter);
    UTIL_NOTUSED(mask);

    char buf[TCP_MSG_BUF_LEN] = {'\0'};
    int buf_len = util_tcp_recv(fd, buf, TCP_MSG_BUF_LEN);
    if (buf_len == 0) {
        log_debug("handle_admin_tcp_recv buf_len==0\n");
        // close fd and client
        ae_del_net_event(base, fd, AE_NET_EVENT_READ | AE_NET_EVENT_WRITE);
        util_tcp_shutdown(fd, 2);
    } else if (buf_len < 0) {
        log_debug("handle_admin_tcp_recv buf_len<0,%d\n", buf_len);
        ae_del_net_event(base, fd, AE_NET_EVENT_READ | AE_NET_EVENT_WRITE);
        util_tcp_shutdown(fd, 2);
    } else {
        log_debug("handle_admin_tcp_recv>>>:%d,%s\n", buf_len, buf);
        hdl_admin_recv_data(fd, buf, buf_len);
    }
}

void hdl_server_accpet(struct reactor_base *base, int fd, void *fn_parameter,
                       int mask) {
    UTIL_NOTUSED(base);
    UTIL_NOTUSED(mask);

    int connfd = util_tcp_accept(fd);
    log_debug("hdl_server_accpet connfd:%d\n", connfd);
    util_set_fd_nonblocking(connfd);
    ae_add_net_event(base, connfd, AE_NET_EVENT_READ, handle_tcp_recv,
                     fn_parameter, "handle_tcp_recv");
}

void hdl_admin_server_accpet(struct reactor_base *base, int fd, void *fn_parameter,
                             int mask) {
    UTIL_NOTUSED(base);
    UTIL_NOTUSED(mask);

    int connfd = util_tcp_accept(fd);
    log_debug("hdl_admin_server_accpet connfd:%d\n", connfd);
    util_set_fd_nonblocking(connfd);
    ae_add_net_event(base, connfd, AE_NET_EVENT_READ, handle_admin_tcp_recv,
                     fn_parameter, "handle_admin_tcp_recv");
}