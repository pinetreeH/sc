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

static http_parser_settings settings;
static int ping_interval;
static int ping_timeout;
static const char *sio_connect_packet;
static int sio_connect_packet_len;
static const char *sio_pong_packet;
static int sio_pong_packet_len;

int valid_transport(const char *transport_str) {
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
    int ret = get_value_of_key(url, KEY_SID, KEY_END_FLAG, sid_str);
    if (ret > 0) {
        info->has_sid = 1;
        strcpy(info->sid, sid_str);
        log_debug("sid:%s\n", sid_str);
    }
    // get transport
    char transport_str[STR_MAX_LEN] = {'\0'};
    ret = get_value_of_key(url, KEY_TRANSPORT, KEY_END_FLAG, transport_str);
    if (ret > 0) {
        info->has_transport = 1;
        strcpy(info->transport, transport_str);
        log_debug("transport:%s\n", transport_str);
    }
}

int parse_http_request(const char *buf, int buf_len,
                       struct http_request_info *info) {
    http_parser *parser = (http_parser *) malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    info->has_sid = info->has_transport = 0;
    parser->data = info;
    int res = http_parser_execute(parser, &settings, buf, buf_len);
    log_debug("http_parser_execute,parsed:%d, buflen:%d\n", res, buf_len);
}

void http_parse_setting_init(void) {
    settings.on_url = parse_url_callback;
}

void websocket_response(const char *req, char *resp, int resp_len) {
    WEBSOCKET_generate_handshake(req, resp, resp_len);
}

int get_transport_config_msg(const char *sid, char *msg) {
    cJSON *root = cJSON_CreateObject();
    cJSON *upgrades_body = cJSON_CreateArray();
    cJSON_AddStringToObject(root, "sid", sid);
    cJSON_AddNumberToObject(root, "pingInterval", ping_interval);
    cJSON_AddNumberToObject(root, "pingTimeout", ping_timeout);
    cJSON_AddItemToObject(root, "upgrades", upgrades_body);
    strcpy(msg, cJSON_PrintUnformatted(root));
    cJSON_Delete(root);
}

void transport_config_init(int interval, int timeout) {
    ping_interval = interval;
    ping_timeout = timeout;
}

int encode(sio_packet_type type, const char *data, int data_len,
           char *encoded_data, int encoded_len) {
}

int eio_encode(eio_packet_type type, const char *data, int data_len,
               char *encoded_data, int encoded_len) {
    if (encoded_len <= data_len)
        return -1;

    int encode_idx = 0;
    if (type == EIO_PACKET_OPEN) {
        encoded_data[encode_idx++] = EIO_PACKET_OPEN;
        for (int i = 0; i < data_len; i++) {
            encoded_data[encode_idx++] = data[i];
        }
    }

    return encode_idx;
}

int websocket_set_msg(const char *data, int data_len, char *dst, int dst_len) {
    return WEBSOCKET_set_content(data, data_len, dst, dst_len);
}

inline const char *get_sio_connect_packet(void) {
    return sio_connect_packet;
}

inline int get_sio_connect_packet_len(void) {
    return sio_connect_packet_len;
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

}

void default_sio_packet_init(void) {
    default_sio_connect_packet_init();
    default_sio_pong_packet_init();
}
