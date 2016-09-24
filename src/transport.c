//
// Created by pinetree on 16-9-14.
//

#include "transport.h"
#include "util.h"
#include "3rd/http_parser/http_parser.h"
#include "3rd/c-websocket/cWebSockets.h"
#include "3rd/cJson/cJSON.h"
#include <string.h>
//#include <regex.h>
#include <stdlib.h>

#define URL_MAX_LEN  256
#define STR_MAX_LEN  256

#define KEY_TRANSPORT "transport="
#define KEY_SID "sid="
#define KEY_END_FLAG '&'

#define TRANSPORT_WEBSOCKET "websocket"

enum eio_packet_type {
    EIO_PACKET_OPEN = '0',
    EIO_PACKET_CLOSE = '1',
    EIO_PACKET_PING = '2',
    EIO_PACKET_PONG = '3',
    EIO_PACKET_MESSAGE = '4',
    EIO_PACKET_UPGRADE = '5',
    EIO_PACKET_NOOP = '6'
};

enum sio_packet_type {
    SIO_PACKET_CONNECT = '0',
    SIO_PACKET_DISCONNECT = '1',
    SIO_PACKET_EVENT = '2',
    SIO_PACKET_ACK = '3',
    SIO_PACKET_ERROR = '4',
    SIO_PACKET_BINARY_EVENT = '5',
    SIO_PACKET_BINARY_ACK = '6'
};

static http_parser_settings settings;
static int ping_interval;
static int ping_timeout;
static const char *sio_connect_packet;
static int sio_connect_packet_len;
static const char *sio_pong_packet;
static int sio_pong_packet_len;

int tra_valid_transport(const char *transport_str) {
    return strcmp(transport_str, TRANSPORT_WEBSOCKET) == 0 ? 1 : -1;
}

static int parse_url_callback(http_parser *parser, const char *buf,
                              size_t len) {
    char url[URL_MAX_LEN] = {'\0'};
    memcpy(url, buf, len);
    log_debug("parse_url_callback,len:%d, url:%s\n", len, url);
    struct http_request_info *info = (struct http_request_info *) parser->data;

    // try to get sid first
    char sid_str[STR_MAX_LEN] = {'\0'};
    int ret = util_get_value_of_key(url, KEY_SID, KEY_END_FLAG, sid_str);
    if (ret > 0) {
        info->has_sid = 1;
        strcpy(info->sid, sid_str);
        log_debug("sid:%s\n", sid_str);
    }
    // get transport
    char transport_str[STR_MAX_LEN] = {'\0'};
    ret = util_get_value_of_key(url, KEY_TRANSPORT, KEY_END_FLAG,
                                transport_str);
    if (ret > 0) {
        info->has_transport = 1;
        strcpy(info->transport, transport_str);
        log_debug("transport:%s\n", transport_str);
    }
}

int tra_parse_http_req(const char *buf, int buf_len,
                       struct http_request_info *info) {
    http_parser *parser = (http_parser *) malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    info->has_sid = info->has_transport = 0;
    parser->data = info;
    int res = http_parser_execute(parser, &settings, buf, buf_len);
    log_debug("http_parser_execute,parsed:%d, buflen:%d\n", res, buf_len);
}

void tra_http_parse_init(void) {
    settings.on_url = parse_url_callback;
}

void tra_ws_resp(const char *req, char *resp, int resp_len) {
    WEBSOCKET_generate_handshake(req, resp, resp_len);
}

int tra_get_conf(const char *sid, char *msg) {
    cJSON *root = cJSON_CreateObject();
    cJSON *upgrades_body = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "sid", sid);
    cJSON_AddNumberToObject(root, "pingInterval", ping_interval);
    cJSON_AddNumberToObject(root, "pingTimeout", ping_timeout);
    cJSON_AddItemToObject(root, "upgrades", upgrades_body);
    strcpy(msg, cJSON_PrintUnformatted(root));
    cJSON_Delete(root);
}

void tra_conf_init(int interval, int timeout) {
    ping_interval = interval;
    ping_timeout = timeout;
}

int tra_encode(tra_sio_packet_type type, const char *data, int data_len,
               char *encoded_data, int encoded_len) {
}

int tra_eio_encode(tra_eio_packet_type type, const char *data, int data_len,
                   char *encoded_data, int encoded_len) {
    if (encoded_len <= data_len)
        return -1;

    int encode_idx = 0;
    if (type == TRA_EIO_PACKET_OPEN) {
        encoded_data[encode_idx++] = EIO_PACKET_OPEN;
        for (int i = 0; i < data_len; i++) {
            encoded_data[encode_idx++] = data[i];
        }
    }

    return encode_idx;
}

int tra_ws_set_content(const char *data, int data_len, char *dst, int dst_len) {
    return WEBSOCKET_set_content(data, data_len, dst, dst_len);
}

int tra_ws_get_content(const char *data, int data_len, char *dst, int dst_len) {
    return WEBSOCKET_get_content(data, data_len, dst, dst_len);
}

inline const char *tra_get_sio_connect_packet(void) {
    return sio_connect_packet;
}

inline int tra_get_sio_connect_packet_len(void) {
    return sio_connect_packet_len;
}

const char *tra_get_sio_pong_packet(void) {
    return sio_pong_packet;
}

int tra_get_sio_pong_packet_len(void) {
    return sio_pong_packet_len;
}

static void default_sio_connect_packet_init(void) {
    char connect_msg[2] = {0};
    connect_msg[0] = EIO_PACKET_MESSAGE;
    connect_msg[1] = SIO_PACKET_CONNECT;
    char websocket_msg[128] = {0};
    sio_connect_packet_len = WEBSOCKET_set_content(connect_msg, 2,
                                                   websocket_msg, 128);
    sio_connect_packet = (char *) mem_malloc(sio_connect_packet_len);
    memcpy(sio_connect_packet, websocket_msg, sio_connect_packet_len);
}

static void default_sio_pong_packet_init(void) {
    char connect_msg[1] = {0};
    connect_msg[0] = EIO_PACKET_PONG;
    char websocket_msg[32] = {0};
    sio_pong_packet_len = WEBSOCKET_set_content(connect_msg, 1,
                                                websocket_msg, 32);
    sio_pong_packet = (char *) mem_malloc(sio_pong_packet_len);
    memcpy(sio_pong_packet, websocket_msg, sio_pong_packet_len);
}

void tra_default_sio_packet_init(void) {
    default_sio_connect_packet_init();
    default_sio_pong_packet_init();
}

tra_eio_packet_type tra_eio_decode(const char *data, int len) {
    tra_eio_packet_type etype = TRA_EIO_PACKET_SCERR;
    char type = data[0];
    switch (type) {
        case EIO_PACKET_OPEN:
            etype = TRA_EIO_PACKET_OPEN;
            break;
        case EIO_PACKET_CLOSE:
            etype = TRA_EIO_PACKET_CLOSE;
            break;
        case EIO_PACKET_PING:
            etype = TRA_EIO_PACKET_PING;
            break;
        case EIO_PACKET_PONG:
            etype = TRA_EIO_PACKET_PONG;
            break;
        case EIO_PACKET_MESSAGE:
            etype = TRA_EIO_PACKET_MESSAGE;
            break;
        case EIO_PACKET_UPGRADE:
            etype = TRA_EIO_PACKET_UPGRADE;
            break;
        case EIO_PACKET_NOOP:
            etype = TRA_EIO_PACKET_NOOP;
            break;
        default:
            break;
    }
    return etype;
}

tra_sio_packet_type tra_sio_decode(const char *data, int len) {
    tra_sio_packet_type stype = TRA_SIO_PACKET_SCERR;
    char type = data[0];
    switch (type) {
        case SIO_PACKET_CONNECT:
            stype = TRA_SIO_PACKET_CONNECT;
            break;
        case SIO_PACKET_DISCONNECT:
            stype = TRA_SIO_PACKET_DISCONNECT;
            break;
        case SIO_PACKET_EVENT:
            stype = TRA_SIO_PACKET_EVENT;
            break;
        case SIO_PACKET_ACK:
            stype = TRA_SIO_PACKET_ACK;
            break;
        case SIO_PACKET_ERROR:
            stype = TRA_SIO_PACKET_ERROR;
            break;
        case SIO_PACKET_BINARY_EVENT:
            stype = TRA_SIO_PACKET_BINARY_EVENT;
            break;
        case SIO_PACKET_BINARY_ACK:
            stype = TRA_SIO_PACKET_BINARY_ACK;
            break;
        default:
            break;
    }
    return stype;
}

int tra_get_ping_timeout(void) {
    return ping_timeout;
}

