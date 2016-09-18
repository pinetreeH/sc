//
// Created by pinetree on 16-9-9.
//

#include "netevent.h"
#include "session.h"
#include "transport.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define TCP_MSG_BUF_LEN 1024
#define TMP_MSG_MAX 512

static inline void show_ip_port(struct sockaddr_in *sockfd) {
    char ip[INET_ADDRSTRLEN] = {0};
    log_debug("client ip:%s,port:%d\n",
              inet_ntop(AF_INET, &sockfd->sin_addr, ip, INET_ADDRSTRLEN),
              ntohs(sockfd->sin_port));
}

static void handle_tcp_recv(struct reactor_base *base, int fd,
                            void *fn_parameter, int mask) {
    NOTUSED_PARAMETER(base);
    NOTUSED_PARAMETER(fn_parameter);
    NOTUSED_PARAMETER(mask);

    char buf[TCP_MSG_BUF_LEN] = {'\0'};
    int rbytes = tcp_recv(fd, buf, TCP_MSG_BUF_LEN);
    if (rbytes <= 0) {
        log_error("handleTcpRecv_recv rbytes<=0,%d\n", rbytes);
        exit(EXIT_FAILURE);
    }
    log_debug("handleTcpRecv_recv:%d,%s\n", rbytes, buf);
    if (is_new_connection(fd, buf, rbytes)) {
        log_debug("is_new_connection,fd:%d\n", fd);
        // this is a new client, parse http request
        struct http_request_info info;
        parse_http_request(buf, rbytes, &info);
        // new client has no sid and transport should be "websocket"
        if (!info.has_sid && valid_transport(info.transport)) {
            char websocket_res[WEBSOCKET_RESPONSE_MAX] = {'\0'};
            // TODO
            websocket_response(buf, websocket_res, WEBSOCKET_RESPONSE_MAX);
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
    }
}

void set_fd_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
}


int init_socket(char *ip, int port) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        log_error("call socket fail!\n");
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int ret = bind(listenfd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        log_error("call bind fail!\n");
        exit(EXIT_FAILURE);
    }
    ret = listen(listenfd, 10);
    if (ret == -1) {
        log_error("call listen fail!\n");
        exit(EXIT_FAILURE);
    }
    return listenfd;
}

void handle_server_accpet(struct reactor_base *base, int fd, void *fd_parameter,
                          int mask) {
    NOTUSED_PARAMETER(base);
    NOTUSED_PARAMETER(fd_parameter);
    NOTUSED_PARAMETER(mask);

    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int connfd = accept(fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
    log_debug("handle_server_accpet connfd:%d\n", connfd);
    show_ip_port(&clientaddr);
    set_fd_nonblocking(connfd);
    reactor_add_net_event(base, connfd, REACTOR_EVENT_READ, handle_tcp_recv,
                          NULL, "handle_tcp_recv");
}

int tcp_send(int fd, const char *data, int len) {
    return send(fd, data, len, 0);
}

int tcp_recv(int fd, char *data, int len) {
    return recv(fd, data, len, 0);
}