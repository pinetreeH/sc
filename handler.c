//
// Created by pinetree on 16-9-19.
//

#include "handler.h"
#include "session.h"
#include "transport.h"
#include "util.h"
#include <string.h>

#define TMP_MSG_MAX 256

int handle_connection_data(int fd, const char *data, int len) {
    if (is_new_connection(fd, data, len)) {
        log_debug("is_new_connection,fd:%d\n", fd);
        // this is a new client, parse http request
        struct http_request_info info;
        parse_http_request(data, len, &info);
        // new client has no sid and transport should be "websocket"
        if (!info.has_sid && valid_transport(info.transport)) {
            char websocket_res[WEBSOCKET_RESPONSE_MAX] = {'\0'};
            // TODO
            websocket_response(data, websocket_res, WEBSOCKET_RESPONSE_MAX);
            tcp_send(fd, websocket_res, strlen(websocket_res));
            // send {sid,upgrade,pingTimeout} ...
            char sid[SID_STR_MAX] = {'\0'};
            make_sid(sid);
            char transport_config_msg[256];
            get_transport_config_msg(sid, transport_config_msg);
            int config_msg_len = strlen(transport_config_msg);
            char encode_msg[TMP_MSG_MAX];
            int encode_data_len = eio_encode(EIO_PACKET_OPEN,
                                             transport_config_msg,
                                             config_msg_len, encode_msg,
                                             TMP_MSG_MAX);

            char websocket_msg[TMP_MSG_MAX];
            int websocket_msg_len = websocket_set_msg(encode_msg,
                                                      encode_data_len,
                                                      websocket_msg,
                                                      TMP_MSG_MAX);

            tcp_send(fd, websocket_msg, websocket_msg_len);
            // after send config msg, we create a new client
            add_new_client(fd, sid);
            // send CONNECT packet
            tcp_send(fd, get_sio_connect_packet(),
                     get_sio_connect_packet_len());
        }
    } else {
        // exist client, parse msg by transport protocol
        log_debug("exist client,fd:%d\n", fd);
        char msg[TMP_MSG_MAX] = {0};
        int msg_len = websocket_get_msg(data, len, msg, TMP_MSG_MAX);
        eio_packet_type etype = eio_decode(msg, msg_len);
        if (etype == EIO_PACKET_PING) {
            tcp_send(fd, get_sio_pong_packet(), get_sio_pong_packet_len());
        } else if (etype == EIO_PACKET_MESSAGE) {
            char client_msg[TMP_MSG_MAX] = {'\0'};
            int client_msg_len = get_msg_from_websocket_data(msg, msg_len,
                                                             client_msg);
            log_debug("EIO_PACKET_MESSAGE content,len%d;%s\n",
                      client_msg_len, client_msg);
        }
    }
}