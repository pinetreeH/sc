//
// Created by pinetree on 16-9-14.
//

#ifndef TRANSPORT_H
#define TRANSPORT_H

#define SID_STR_MAX 32
#define TRANSPORT_STR_MAX 32
#define WEBSOCKET_RESPONSE_MAX 1024

typedef enum _eio_packet_type {
    EIO_PACKET_ERR = -1,
    EIO_PACKET_OPEN = '0',
    EIO_PACKET_CLOSE = '1',
    EIO_PACKET_PING = '2',
    EIO_PACKET_PONG = '3',
    EIO_PACKET_MESSAGE = '4',
    EIO_PACKET_UPGRADE = '5',
    EIO_PACKET_NOOP = '6'
} eio_packet_type;

typedef enum _sio_packet_type {
    SIO_PACKET_ERR = -1,
    SIO_PACKET_CONNECT = '0',
    SIO_PACKET_DISCONNECT = '1',
    SIO_PACKET_EVENT = '2',
    SIO_PACKET_ACK = '3',
    SIO_PACKET_ERROR = '4',
    SIO_PACKET_BINARY_EVENT = '5',
    SIO_PACKET_BINARY_ACK = '6'
} sio_packet_type;


struct http_request_info {
    char sid[SID_STR_MAX];
    char transport[TRANSPORT_STR_MAX];
    int has_sid;
    int has_transport;
};

extern int valid_transport(const char *transport_str);

extern void http_parse_setting_init(void);

extern int parse_http_request(const char *buf, int buf_len,
                              struct http_request_info *info);

extern void websocket_response(const char *req, char *resp, int resp_len);


extern int get_transport_config_msg(const char *sid, char *msg);

extern void transport_config_init(int ping_interval, int ping_timeout);

extern int encode(sio_packet_type type, const char *data, int data_len,
                  char *encoded_data, int encoded_len);

extern int eio_encode(eio_packet_type type, const char *data, int data_len,
                      char *encoded_data, int encoded_len);

extern eio_packet_type eio_decode(const char *data, int data_len);

extern int websocket_set_msg(const char *data, int data_len, char *dst,
                             int dst_len);

extern int websocket_get_msg(const char *data, int data_len, char *dst,
                             int dst_len);

extern void default_sio_packet_init(void);

extern const char *get_sio_connect_packet(void);

extern int get_sio_connect_packet_len(void);

extern const char *get_sio_pong_packet(void);

extern int get_sio_pong_packet_len(void);

extern int get_ping_timeout(void);

#endif
