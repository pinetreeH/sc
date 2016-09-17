//
// Created by pinetree on 16-9-14.
//

#ifndef TRANSPORT_H
#define TRANSPORT_H

#define SID_STR_MAX 32
#define TRANSPORT_STR_MAX 32
#define WEBSOCKET_RESPONSE_MAX 1024

typedef enum _packet_type {
    PACKET_CONNECT = 0,
    PACKET_DISCONNECT = 1,
    PACKET_EVENT = 2,
    PACKET_ACK = 3,
    PACKET_ERROR = 4,
    PACKET_BINARY_EVENT = 5,
    PACKET_BINARY_ACK = 6
} packet_type;


struct http_request_info {
    char sid[SID_STR_MAX];
    char transport[TRANSPORT_STR_MAX];
    int has_sid;
    int has_transport;
};

int valid_transport(const char *transport_str);

void http_parse_setting_init(void);

int parse_http_request(const char *buf, int buf_len,
                       struct http_request_info *info);

void websocket_response(const char *req, char *resp, int resp_len);


int get_transport_config_msg(const char *sid, char *msg);

void transport_config_init(int ping_interval, int ping_timeout);

int encode(packet_type type, const char *data, int data_len,
           char *encoded_data, int encoded_len);

#endif
