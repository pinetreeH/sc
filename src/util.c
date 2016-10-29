//
// Created by pinetree on 16-9-9.
//

#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LOG_MSG_MAX_LEN  1024

void log_debug(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "DEBUG";
    printf("%s> %s\n", prefix, msg);
}

void log_info(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "INFO";
    printf("%s> %s\n", prefix, msg);
}

void log_err(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "ERROR";
    printf("%s> %s\n", prefix, msg);
}


// test: /socket.io/?EIO=3&transport=polling&t=1473479272828-0
int util_get_value_of_key(const char *str, const char *key, const char end_flag,
                          char *value) {
    int idx = 0;
    char *i = NULL;
    if ((i = strstr(str, key)) != NULL) {
        char *j = i + strlen(key);
        while (*j != end_flag && *j != '\0') {
            value[idx] = *j;
            j++;
            idx++;
        }
        value[idx] = '\0';
    }
    return idx;
}

int util_get_timestamp(void) {
    return time(NULL);
}

int util_gen_sid_by_fd(int fd,char *sid) {
    int port = 0;
    char ip[INET_ADDRSTRLEN] = {'\0'};
    util_get_fd_ip_port(fd,ip,&port);
    strcpy(sid,ip);
    strcat(sid,":");
    char port_str[10] = {0};
    snprintf(port_str,sizeof(port_str),"%d",port);
    strcat(sid,port_str);
}

void *mem_malloc(int size) {
    return malloc(size);
}

void mem_free(void *data) {
    if (data) {
        free(data);
    }
}

void *mem_calloc(int num, int size) {
    return calloc(num, size);
}

int util_tcp_send(int fd, const char *data, int len) {
    return send(fd, data, len, 0);
}

int util_tcp_recv(int fd, char *data, int len) {
    return recv(fd, data, len, 0);
}

int util_tcp_shutdown(int fd, int how) {
    return shutdown(fd, how);
}

void util_set_fd_nonblocking(int fd) {
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
}

int util_get_fd_ip_port(int fd,char *ip,int *port){
    struct sockaddr addr;
    socklen_t len = sizeof(struct sockaddr);
    if(getpeername(fd,&addr,&len) != -1){
        struct sockaddr_in *sin = (struct sockaddr_in*)(&addr);
        inet_ntop(AF_INET, &sin->sin_addr, ip, INET_ADDRSTRLEN);
        *port = ntohs(sin->sin_port);
        return 0;
    }
    return -1;
}

int util_init_socket(char *ip, int port) {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        log_err("call socket fail!\n");
        exit(EXIT_FAILURE);
    }
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    int ret = bind(listenfd, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        log_err("call bind fail!\n");
        exit(EXIT_FAILURE);
    }
    ret = listen(listenfd, 10);
    if (ret == -1) {
        log_err("call listen fail!\n");
        exit(EXIT_FAILURE);
    }
    return listenfd;
}
