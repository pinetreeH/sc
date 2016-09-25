//
// Created by pinetree on 16-9-14.
//

#ifndef SC_TRANSPORT_TRA_H
#define SC_TRANSPORT_TRA_H

#define TRA_SID_STR_MAX 48
#define TRA_STR_MAX 32
#define TRA_WS_RESP_MAX 1024

typedef enum _tra_eio_packet_type {
    TRA_EIO_PACKET_SCERR = -1,
    TRA_EIO_PACKET_OPEN = 0,
    TRA_EIO_PACKET_CLOSE = 1,
    TRA_EIO_PACKET_PING = 2,
    TRA_EIO_PACKET_PONG = 3,
    TRA_EIO_PACKET_MESSAGE = 4,
    TRA_EIO_PACKET_UPGRADE = 5,
    TRA_EIO_PACKET_NOOP = 6
} tra_eio_packet_type;

typedef enum _tra_sio_packet_type {
    TRA_SIO_PACKET_SCERR = -1,
    TRA_SIO_PACKET_CONNECT = 0,
    TRA_SIO_PACKET_DISCONNECT = 1,
    TRA_SIO_PACKET_EVENT = 2,
    TRA_SIO_PACKET_ACK = 3,
    TRA_SIO_PACKET_ERROR = 4,
    TRA_SIO_PACKET_BINARY_EVENT = 5,
    TRA_SIO_PACKET_BINARY_ACK = 6
} tra_sio_packet_type;

struct http_request_info {
    char sid[TRA_SID_STR_MAX];
    char transport[TRA_STR_MAX];
    int has_sid;
    int has_transport;
};

extern int tra_valid_transport(const char *transport_str);

extern void tra_http_parse_init(void);

extern int tra_parse_http_req(const char *buf, int buf_len,
                              struct http_request_info *info);

extern void tra_ws_resp(const char *req, char *resp, int resp_len);


extern int tra_get_conf(const char *sid, char *msg);

extern void tra_conf_init(int ping_interval, int ping_timeout);

extern int tra_sio_encode(tra_sio_packet_type type,
                          const char *nsp, int nsp_len,
                          const char *event, int event_len,
                          const char *msg, int msg_len,
                          char *encoded_data, int encoded_len);

extern int tra_eio_encode(tra_eio_packet_type type, const char *data, int len,
                          char *encoded_data, int encoded_len);

extern tra_eio_packet_type tra_eio_decode(const char *data, int len);

extern tra_sio_packet_type tra_sio_decode(const char *data, int len);

extern int tra_ws_set_content(const char *data, int data_len, char *dst,
                              int dst_len);

extern int tra_ws_get_content(const char *data, int data_len, char *dst,
                              int dst_len);

extern void tra_default_sio_packet_init(void);

extern const char *tra_get_sio_connect_packet(void);

extern int tra_get_sio_connect_packet_len(void);

extern const char *tra_get_sio_pong_packet(void);

extern int tra_get_sio_pong_packet_len(void);

extern int tra_get_ping_timeout(void);

#endif
