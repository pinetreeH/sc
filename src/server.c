#include "server.h"
#include "reactor.h"
#include "session.h"
#include "transport.h"
#include "handler.h"
#include "nsp.h"
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

struct server srv;

int main(int argc, char **args) {
    int capacity = 128;

    srv.ae = ae_init(capacity);
    srv.nsp = nsp_new();
    srv.ses = ses_init(capacity);
    srv.ping_interval = 3000;
    srv.ping_timeout = 5000;
    srv.port = 5074;
    srv.admin_port = 9527;
    if (!srv.ae || !srv.nsp || !srv.ses)
        exit(EXIT_FAILURE);

    tra_conf_init(srv.ping_interval, srv.ping_timeout);
    tra_http_parse_init();
    tra_default_sio_packet_init();

    int sockfd = util_init_socket("localhost", srv.port);
    util_set_fd_nonblocking(sockfd);
    ae_add_net_event(srv.ae, sockfd, AE_NET_EVENT_READ,
                     hdl_server_accpet, (void *) &srv,
                     "hdl_server_accpet");

    int admin_sockfd = util_init_socket("localhost", srv.admin_port);
    util_set_fd_nonblocking(admin_sockfd);
    ae_add_net_event(srv.ae, admin_sockfd, AE_NET_EVENT_READ,
                     hdl_admin_server_accpet, (void *) &srv,
                     "hdl_admin_server_accpet");

    struct handler_if *msg_handler = NULL;
    msg_handler = (struct handler_if *) mem_malloc(sizeof(struct handler_if));
    msg_handler->on_connect = sc_on_connect;
    msg_handler->on_disconnect = sc_on_disconnect;
    msg_handler->on_event = sc_on_event;
    msg_handler->on_ack = sc_on_ack;
    msg_handler->on_error = sc_on_error;
    msg_handler->on_binary_event = sc_on_binary_event;
    msg_handler->on_binary_ack = sc_on_binary_ack;
    srv.handler = msg_handler;

    log_debug("current_timestamp:%d", util_get_timestamp());
    //ae_add_time_event(srv.ae, foobar, "2", "foobar", 1, 3);
    ae_add_time_event(srv.ae, ses_handle_timeout_client, (void *) &srv,
                      "ses_handle_timeout_client", 1,
                      AE_TIME_EVENT_REPEAT_INFINITE);

    ae_run(srv.ae, AE_NET_EVENT | AE_TIME_EVENT);
    ae_del(srv.ae);
    nsp_del(srv.nsp);
    ses_del(srv.ses);

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
    struct client *c = ses_get_client_by_fd(srv.ses, fd);
    const char *event = "\"news\"";
    char client_msg[256] = {0};
    sprintf(client_msg, "{\"hello\":\"your sid:%s\"}", client_sid(c));
    char bro_msg[256] = {0};
    sprintf(bro_msg, "{\"hello_all\":\"welcome new client:%s\"}", client_sid(c));
    hdl_emit(c, event, strlen(event), client_msg, strlen(client_msg));
    hdl_broadcast(srv.ses, &c, 1, event, strlen(event), bro_msg, strlen(bro_msg));
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

