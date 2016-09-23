//
// Created by pinetree on 16-9-19.
//

#include "handler.h"
#include "session.h"
#include "transport.h"
#include "util.h"
#include <string.h>

#define TMP_MSG_MAX 256

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
        util_gen_sid_by_fd(fd,sid);
        char transport_config_msg[256];
        tra_get_conf(sid, transport_config_msg);
        int config_msg_len = strlen(transport_config_msg);
        char encode_msg[TMP_MSG_MAX];
        int encode_data_len = tra_eio_encode(EIO_PACKET_OPEN,
                                             transport_config_msg,
                                             config_msg_len, encode_msg,
                                             TMP_MSG_MAX);

        char websocket_msg[TMP_MSG_MAX];
        int websocket_msg_len = tra_ws_set_content(encode_msg,
                                                   encode_data_len,
                                                   websocket_msg,
                                                   TMP_MSG_MAX);

        util_tcp_send(fd, websocket_msg, websocket_msg_len);
        // after send config msg, we create a new client
        ses_add_new_client(fd,sid);
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
    util_tcp_send(fd, tra_get_sio_pong_packet(), get_sio_pong_packet_len());

}

static int handle_eio_msg_packet(int fd, const char *data, int len) {
    char client_msg[TMP_MSG_MAX] = {'\0'};
    int client_msg_len = util_get_msg_from_ws_data(data, len, client_msg);
    log_debug("EIO_PACKET_MESSAGE content,len%d;%s\n", client_msg_len,
              client_msg);
}

static int handle_client_msg(int fd, const char *data, int len) {
    char msg[TMP_MSG_MAX] = {0};
    int msg_len = tra_ws_get_content(data, len, msg, TMP_MSG_MAX);

    tra_eio_packet_type etype = eio_decode(msg, msg_len);
    if (etype == EIO_PACKET_PING) {
        handle_eio_ping_packet(fd);
    } else if (etype == EIO_PACKET_MESSAGE) {
        handle_eio_msg_packet(fd, msg, msg_len);
    }
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

int hdl_recv_close(int fd){
    return ses_del_client_by_fd(fd);
}

extern int hdl_recv_err(int fd){
    return ses_del_client_by_fd(fd);
}