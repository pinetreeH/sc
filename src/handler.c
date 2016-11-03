//
// Created by pinetree on 16-9-19.
//

#include "handler.h"
#include "handler_if.h"
#include "session.h"
#include "room.h"
#include "nsp.h"
#include "transport.h"
#include "reactor.h"
#include "client.h"
#include "util.h"
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

static int handle_sio_msg_packet(struct server *srv,
                                 struct handler_if *handler, int fd,
                                 const char *data, int len) {
    log_debug("EIO_PACKET_MESSAGE content,%s\n", data);
    tra_sio_packet_type sio_type = tra_sio_decode(data, len);

    switch (sio_type) {
        case TRA_SIO_PACKET_CONNECT:
            handler->on_connect(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_DISCONNECT:
            handler->on_disconnect(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_EVENT:
            handler->on_event(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ACK:
            handler->on_ack(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ERROR:
            handler->on_error(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_EVENT:
            handler->on_binary_event(srv, fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_ACK:
            handler->on_binary_ack(srv, fd, data + 1, len - 1);
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
            handle_sio_msg_packet(srv, srv->handler, fd, msg + 1, msg_len - 1);
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

int hdl_admin_recv_data(struct server *srv, int fd, const char *data, int len) {
    log_debug("hdl_admin_recv_data,fd:%d,data:%s\n", fd, data);
    char test_admin_msg[TMP_MSG_MAX] = {'\0'};
    if (len > 0) {
        int idx = 0;
        for (; idx < len && idx < TMP_MSG_MAX - 1 && data[idx] != '\r'; idx++) {
            test_admin_msg[idx] = data[idx];
        }
        // demo admin command : pub room_name msg
        // get cmd
        char cmd[TMP_MSG_MAX] = {'\0'};
        int tmp_idx = 0;
        for (; tmp_idx < idx && test_admin_msg[tmp_idx] != ' '; tmp_idx++) {
            cmd[tmp_idx] = test_admin_msg[tmp_idx];
        }
        if (strcmp(cmd, "pub") != 0) {
            log_debug("invalid demo command!\n");
            return -1;
        }

        tmp_idx++;
        char parameter[TMP_MSG_MAX] = {'\0'};
        for (int i = 0; tmp_idx < idx && test_admin_msg[tmp_idx] != ' '; tmp_idx++, i++) {
            parameter[i] = test_admin_msg[tmp_idx];
        }
        tmp_idx++;
        char msg[TMP_MSG_MAX] = {'\0'};
        for (int i = 0; tmp_idx < idx; tmp_idx++, i++) {
            msg[i] = test_admin_msg[tmp_idx];
        }

        const char *event = "\"news\"";
        char bro_msg[TMP_MSG_MAX * 2] = {0};
        sprintf(bro_msg, "{\"admin\":\"msg:%s\"}", msg);
        log_debug("admin_msg:%s\n", bro_msg);
        hdl_room_broadcast(srv->nsp, parameter, NULL, 0, event, strlen(event), bro_msg, strlen(bro_msg));
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

    log_debug("hdl_emit data len:%d, event:%s\n", encode_len, event);
    client_send_data(c, encode_data, encode_len);
    return 0;
}

int hdl_broadcast(struct session *s,
                  struct client **except_clients, int client_size,
                  const char *event, int event_len,
                  const char *msg, int len) {

    if (!s || !event || !event_len || !msg || !len)
        return -1;

    struct client *c = NULL;
    struct session_iterator *ses_it = NULL;
    ses_it = ses_iterator_new(s);
    while (ses_valid_iterator(ses_it)) {
        ses_next_client(s, &ses_it, &c);
        if (c && !in_except_list(except_clients, client_size, c))
            hdl_emit(c, event, event_len, msg, len);
    }
    ses_iterator_del(ses_it);

    return 0;
}

int hdl_room_broadcast(struct nsp *n, const char *room_name,
                       struct client **except_clients, int client_size,
                       const char *event, int event_len,
                       const char *msg, int len) {
    if (!n || !room_name || !event || !msg)
        return -1;

    struct room *r = NULL;
    r = nsp_get_room(n, room_name);
    if (!r)
        return -1;

    struct client *c = NULL;
    struct room_iterator *it = NULL;
    it = room_iterator_new(r);
    while (room_valid_iterator(it)) {
        room_next_client(r, &it, &c);
        if (c && !in_except_list(except_clients, client_size, c))
            hdl_emit(c, event, event_len, msg, len);
    }
    room_iterator_del(it);

    return 0;
}

int hdl_jion_room(struct nsp *n, const char *room_name, struct client *c) {
    if (!n || !room_name || !c)
        return -1;
    struct room *r = NULL;
    r = nsp_get_room(n, room_name);
    if (!r) {
        r = room_new(room_name);
        nsp_add_room(n, r);
    }

    return room_jion(r, c);
}

int hdl_leave_room(struct nsp *n, const char *room_name, struct client *c) {
    if (!n || !room_name || !c)
        return -1;
    struct room *r = NULL;
    r = nsp_get_room(n, room_name);
    if (!r)
        return 0;

    room_leave(r, c);
    if (room_client_number(r) == 0)
        nsp_del_room(n, r);

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
    UTIL_NOTUSED(mask);
    struct server *srv = (struct server *) fn_parameter;

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
        hdl_admin_recv_data(srv, fd, buf, buf_len);
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