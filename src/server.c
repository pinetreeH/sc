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
static void sc_on_connect(struct server *srv, int fd, const char *data, int len);

static void sc_on_disconnect(struct server *srv, int fd, const char *data, int len);

static void sc_on_event(struct server *srv, int fd, const char *data, int len);

static void sc_on_ack(struct server *srv, int fd, const char *data, int len);

static void sc_on_error(struct server *srv, int fd, const char *data, int len);

static void sc_on_binary_event(struct server *srv, int fd, const char *data, int len);

static void sc_on_binary_ack(struct server *srv, int fd, const char *data, int len);

static void sc_login(struct server *srv, int fd, const char *data, int len);

static void sc_msg(struct server *srv, int fd, const char *data, int len);


int main(int argc, char **args) {
    int capacity = 128;

    struct server srv;
    srv.ae = ae_init(capacity);
    srv.nsp = nsp_new();
    srv.ses = ses_init(capacity);
    srv.ping_interval = 30000;
    srv.ping_timeout = 50000;
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

void sc_on_connect(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io connect packet, fd:%d, data:%s", fd, data);
}

void sc_on_disconnect(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io disconnect packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_event(struct server *srv, int fd, const char *data, int len) {
    log_debug("******* socket.io event packet, fd:%d, data:%s\n", fd, data);
    // if login event
    char *s = NULL;
    s = strstr(data, "login");
    if (s) {
        sc_login(srv, fd, data, len);
        return;
    }

    s = strstr(data, "cmsg");
    if (s) {
        sc_msg(srv,fd,data,len);
        return;
    }

}

void sc_msg(struct server *srv, int fd, const char *data, int len){
    log_debug("******* socket.io event packet,msg, fd:%d, data:%s,len:%d\n", fd, data,len);
    char msg[512] = {'\0'};
    char *s = strstr(data,",");
    s+=2;
    int msg_size = 0;
    while( s && *s != '\"'){
        msg[msg_size++] = *s;
        s++;
    }
    log_debug("******* socket.io event packet,cmsg, data:%s,len:%d\n", msg,msg_size);

    struct client *c = ses_get_client_by_fd(srv->ses, fd);
    const char *room_name = "yy";
    hdl_jion_room(srv->nsp, room_name, c);
    const char *room_event = "\"bro\"";
    char room_msg[512] = {0};
    sprintf(room_msg, "{\"bro\":\"client(%s) Say:%s\"}", client_sid(c), msg);
    hdl_room_broadcast(srv->nsp, srv->ae, room_name, &c, 1, room_event, strlen(room_event),
                       room_msg, strlen(room_msg));

}

void sc_login(struct server *srv, int fd, const char *data, int len) {
    UTIL_NOTUSED(len);
    log_debug("******* socket.io event packet,login, fd:%d, data:%s\n", fd, data);
    struct client *c = ses_get_client_by_fd(srv->ses, fd);
    const char *event = "\"msg\"";
    char client_msg[256] = {0};
    sprintf(client_msg, "{\"msg\":\"your sid:%s\"}", client_sid(c));
    char bro_msg[256] = {0};
    sprintf(bro_msg, "{\"msg\":\"welcome new client:%s\"}", client_sid(c));
    hdl_emit(c, srv->ae, event, strlen(event), client_msg, strlen(client_msg));
    hdl_broadcast(srv->ses, srv->ae, &c, 1, event, strlen(event), bro_msg, strlen(bro_msg));

    const char *room_name = "yy";
    hdl_jion_room(srv->nsp, room_name, c);
    const char *room_event = "\"bro\"";
    char room_msg[256] = {0};
    sprintf(room_msg, "{\"bro\":\"welcome to yy room:%s\"}", client_sid(c));
    hdl_room_broadcast(srv->nsp, srv->ae, room_name, NULL, 0, room_event, strlen(room_event),
                       room_msg, strlen(room_msg));
}

void sc_on_ack(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io ack packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_error(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io error packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_binary_event(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io binary event packet, fd:%d, data:%s\n", fd, data);
}

void sc_on_binary_ack(struct server *srv, int fd, const char *data, int len) {
    log_debug("socket.io binary ack packet, fd:%d, data:%s\n", fd, data);
}

