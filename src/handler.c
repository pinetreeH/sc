//
// Created by pinetree on 16-9-19.
//

#include "handler.h"
#include "handler_if.h"
#include "session.h"
#include "transport.h"
#include "hashmap.h"
#include "client.h"
#include "util.h"
#include <string.h>
#include <stdio.h>

#define TMP_MSG_MAX 256

//static hashmap *nsp_to_handler;
static struct handler_if msg_handler;

static int handle_new_connection(int fd, const char *data, int len) {
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
        char transport_config_msg[256];
        tra_get_conf(sid, transport_config_msg);
        int config_msg_len = strlen(transport_config_msg);
        char encode_msg[TMP_MSG_MAX];
        int encode_data_len = tra_eio_encode(TRA_EIO_PACKET_OPEN,
                                             transport_config_msg,
                                             config_msg_len, encode_msg,
                                             TMP_MSG_MAX);

        util_tcp_send(fd, encode_msg, encode_data_len);
        // after send config msg, we create a new client
        ses_add_new_client(fd, sid);
        // send CONNECT packet
        util_tcp_send(fd, tra_get_sio_connect_packet(),
                      tra_get_sio_connect_packet_len());
    }
    return 0;
}

static int handle_eio_ping_packet(int fd) {
    // update client heartbeat
    ses_update_client_heartbeat_by_fd(fd);
    // send PONG packet
    util_tcp_send(fd, tra_get_sio_pong_packet(), tra_get_sio_pong_packet_len());

}

static int handle_sio_msg_packet(int fd, const char *data, int len) {
    char client_msg[TMP_MSG_MAX] = {'\0'};
    log_debug("EIO_PACKET_MESSAGE content,%s\n", data);
    tra_sio_packet_type sio_type = tra_sio_decode(data, len);
    switch (sio_type) {
        case TRA_SIO_PACKET_CONNECT:
            msg_handler.on_connect(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_DISCONNECT:
            msg_handler.on_disconnect(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_EVENT:
            msg_handler.on_event(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ACK:
            msg_handler.on_ack(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_ERROR:
            msg_handler.on_error(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_EVENT:
            msg_handler.on_binary_event(fd, data + 1, len - 1);
            break;
        case TRA_SIO_PACKET_BINARY_ACK:
            msg_handler.on_binary_ack(fd, data + 1, len - 1);
            break;
        default:
            break;
    }
    return 0;
}

static int handle_client_msg(int fd, const char *data, int len) {
    char msg[TMP_MSG_MAX] = {0};
    int msg_len = tra_ws_get_content(data, len, msg, TMP_MSG_MAX);

    tra_eio_packet_type etype = tra_eio_decode(msg, msg_len);
    switch (etype) {
        case TRA_EIO_PACKET_OPEN:
            break;
        case TRA_EIO_PACKET_CLOSE:
            break;
        case TRA_EIO_PACKET_PING:
            handle_eio_ping_packet(fd);
            break;
        case TRA_EIO_PACKET_PONG: // this case will not happen!
            break;
        case TRA_EIO_PACKET_MESSAGE:
            handle_sio_msg_packet(fd, msg + 1, msg_len - 1);
            break;
        case TRA_EIO_PACKET_UPGRADE:
            break;
        case TRA_EIO_PACKET_NOOP:
            break;
        default:
            break;
    }
    return 0;
}

int hdl_recv_data(int fd, const char *data, int len) {
    if (ses_new_connection(fd, data, len)) {
        log_debug("ses_new_connection,fd:%d\n", fd);
        handle_new_connection(fd, data, len);
    } else {
        // exist client, parse msg by transport protocol
        log_debug("exist client,fd:%d\n", fd);
        handle_client_msg(fd, data, len);
    }
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
        hdl_broadcast(NULL, 0, event, strlen(event), bro_msg, strlen(bro_msg));
    }
}

int hdl_recv_close(int fd) {
    return ses_del_client_by_fd(fd);
}

int hdl_recv_err(int fd) {
    return ses_del_client_by_fd(fd);
}

int hdl_register_handler(const char *nsp, struct handler_if *h) {
    UTIL_NOTUSED(nsp);
    if (!h)
        return -1;

    msg_handler = *h;
    return 0;
}

int hdl_emit(struct client *c, const char *event, int event_len,
             const char *msg, int len) {
    if (!c || !event || !msg || len <= 0 || c->fd <= 0)
        return -1;
    char encode_data[TRA_WS_RESP_MAX] = {0};

    int encode_len = -1;
    encode_len = tra_sio_encode(TRA_SIO_PACKET_EVENT, NULL, 0, event, event_len, msg, len,
                                encode_data, TRA_WS_RESP_MAX);

    log_debug("hdl_emit data len:%d\n", encode_len);
    util_tcp_send(c->fd, encode_data, encode_len);

    return 0;
}

static inline int in_except_list(struct client **except_clients,
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

int hdl_broadcast(struct client **except_clients, int client_size,
                  const char *event, int event_len,
                  const char *msg, int len) {
    int size = 0;
    struct client **cs = ses_get_clients(&size);
    int idx = 0;
    struct client *c = NULL;
    while (size > 0) {
        c = cs[idx++];
        if (c && c->fd > 0) {
            if (!in_except_list(except_clients, client_size, c)) {
                hdl_emit(c, event, event_len, msg, len);
            }
            size--;
        }
    }
    return 0;
}

int hdl_room_broadcast(struct client **except_clients, int client_size,
                       const char *event, int event_len,
                       const char *msg, int len,
                       const char *room_name) {
    if (!room_name || !event || !msg)
        return -1;

    hashmap *room = NULL;
    room = ses_get_room(room_name);
    if (!room)
        return 0;

    struct client *c = NULL;
    hashmap_iterator it = hashmap_get_iterator(room);
    while (hashmap_valid_iterator(it)) {
        it = hashmap_next(it, (void **) &c, NULL);
        if (c && c->fd > 0 && !in_except_list(except_clients, client_size, c))
            hdl_emit(c, event, event_len, msg, len);
    }

    return 0;
}

//int hdl_init(void) {
//    // TODO
//    nsp_to_handler = hashmap_init(128, hashmap_strkey_cmp,
//                                  NULL, NULL, NULL,
//                                  hashmap_strkey_hashindex);
//    if (!nsp_to_handler)
//        return -1;
//
//    return 0;
//}