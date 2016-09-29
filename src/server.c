#include "reactor.h"
#include "netevent.h"
#include "session.h"
#include "transport.h"
#include "handler.h"
#include "handler_if.h"
#include "util.h"
#include "client.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// example function declare
static void sc_on_connect(int fd, const char *data, int len);

static void sc_on_disconnect(int fd, const char *data, int len);

static void sc_on_event(int fd, const char *data, int len);

static void sc_on_ack(int fd, const char *data, int len);

static void sc_on_error(int fd, const char *data, int len);

static void sc_on_binary_event(int fd, const char *data, int len);

static void sc_on_binary_ack(int fd, const char *data, int len);

static void foobar(struct reactor_base *ae, void *data) {
    UTIL_NOTUSED(ae);
    log_debug("foobar_%s,%d\n", (char *) data, util_get_timestamp());
}

int main(int argc, char **args) {
    int capacity = 128;
    struct reactor_base *base = NULL;
    base = ae_init(capacity);
    if (!base)
        exit(EXIT_FAILURE);

    int ping_interval = 10000;
    int ping_timeout = 20000;
    tra_conf_init(ping_interval, ping_timeout);
    char *ip = "127.0.0.1";
    int port = 5074;
    int admin_port = 9527;
    int sockfd = net_init_socket(ip, port);
    util_set_fd_nonblocking(sockfd);
    ae_add_net_event(base, sockfd, AE_NET_EVENT_READ,
                     net_server_accpet, NULL,
                     "net_server_accpet");

    int admin_sockfd = net_init_socket(ip, admin_port);
    util_set_fd_nonblocking(admin_sockfd);
    ae_add_net_event(base, admin_sockfd, AE_NET_EVENT_READ,
                     net_admin_server_accpet, NULL,
                     "net_admin_server_accpet");

    tra_http_parse_init();
    tra_default_sio_packet_init();
    ses_init(capacity);

    struct handler_if msg_handler;
    msg_handler.on_connect = sc_on_connect;
    msg_handler.on_disconnect = sc_on_disconnect;
    msg_handler.on_event = sc_on_event;
    msg_handler.on_ack = sc_on_ack;
    msg_handler.on_error = sc_on_error;
    msg_handler.on_binary_event = sc_on_binary_event;
    msg_handler.on_binary_ack = sc_on_binary_ack;
    hdl_register_handler(NULL, &msg_handler);

    log_debug("current_timestamp:%d", util_get_timestamp());
    //ae_add_time_event(base, foobar, "2", "foobar", 1, 3);
    ae_add_time_event(base, ses_handle_timeout_client, (void *) ping_timeout,
                      "ses_handle_timeout_client", 1, AE_TIME_EVENT_REPEAT_INFINITE);

    ae_run(base, AE_NET_EVENT | AE_TIME_EVENT);
    ae_del(base);

    return EXIT_SUCCESS;
}

void sc_on_connect(int fd, const char *data, int len) {
    log_debug("socket.io connect packet, fd:%d, data:%s", fd, data);
}

void sc_on_disconnect(int fd, const char *data, int len) {
    log_debug("socket.io disconnect packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_event(int fd, const char *data, int len) {
    log_debug("socket.io event packet, fd:%d, data:%s\n", fd, data);
    struct client *c = ses_get_client_by_fd(fd);
    char *event = "\"news\"";
    char client_msg[256] = {0};
    sprintf(client_msg, "{\"hello\":\"your sid:%s\"}", c->sid);
    char bro_msg[256] = {0};
    sprintf(bro_msg, "{\"hello_all\":\"welcome new client:%s\"}", c->sid);
    hdl_emit(c, event, strlen(event), client_msg, strlen(client_msg));
    hdl_broadcast(&c, 1, event, strlen(event), bro_msg, strlen(bro_msg));
}

void sc_on_ack(int fd, const char *data, int len) {
    log_debug("socket.io ack packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_error(int fd, const char *data, int len) {
    log_debug("socket.io error packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_binary_event(int fd, const char *data, int len) {
    log_debug("socket.io binary event packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_binary_ack(int fd, const char *data, int len) {
    log_debug("socket.io binary ack packet, fd:%d, data:%s\n", fd, data);
}

