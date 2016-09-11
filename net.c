//
// Created by pinetree on 16-9-9.
//

#include "net.h"
#include "util.h"
#include "parse_http.h"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define TCP_MSG_BUF_LEN 1024

static inline void show_ip_port(struct sockaddr_in *sockfd) {
    char ip[INET_ADDRSTRLEN] = {0};
    log_debug("client ip:%s,port:%d\n",
              inet_ntop(AF_INET, &sockfd->sin_addr, ip, INET_ADDRSTRLEN),
              ntohs(sockfd->sin_port));
}

static void handle_tcp_recv(struct reactor_base *base, int fd,
                            void *fn_parameter, int mask) {
    REACTOR_NOTUSED_PARAMETER(base);
    REACTOR_NOTUSED_PARAMETER(fn_parameter);
    REACTOR_NOTUSED_PARAMETER(mask);

    char buf[TCP_MSG_BUF_LEN] = {'\0'};
    int rbytes = recv(fd, buf, TCP_MSG_BUF_LEN, 0);
    if (rbytes <= 0) {
        log_error("handleTcpRecv_recv rbytes<=0,%d\n", rbytes);
        exit(EXIT_FAILURE);
    }
    log_debug("handleTcpRecv_recv:%d,%s\n", rbytes, buf);
    // parse_recv
    parse_http_request(buf, rbytes);
}

inline void set_fd_nonblocking(int fd) {
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

void handle_tcp_accpet(struct reactor_base *base, int fd, void *fd_parameter,
                       int mask) {
    REACTOR_NOTUSED_PARAMETER(base);
    REACTOR_NOTUSED_PARAMETER(fd_parameter);
    REACTOR_NOTUSED_PARAMETER(mask);

    struct sockaddr_in clientaddr;
    socklen_t clientaddr_len = sizeof(clientaddr);
    int connfd = accept(fd, (struct sockaddr *) &clientaddr, &clientaddr_len);
    log_debug("handle_tcp_accpet connfd:%d\n", connfd);
    show_ip_port(&clientaddr);
    set_fd_nonblocking(connfd);
    reactor_add_net_event(base, connfd, REACTOR_EVENT_READ, handle_tcp_recv,
                          NULL, "handle_tcp_recv");
}

