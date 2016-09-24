//
// Created by pinetree on 16-9-9.
//

#ifndef SC_UTIL_UTIL_H
#define SC_UTIL_UTIL_H

#define UTIL_NOTUSED(p) ((void) p)

extern void log_debug(const char *fmt, ...);

extern void log_info(const char *fmt, ...);

extern void log_err(const char *fmt, ...);

extern int util_get_value_of_key(const char *str, const char *key,
                                 const char end_flag, char *value);

extern int util_get_timestamp(void);

extern int util_gen_sid_by_fd(int fd, char *sid);

extern void *mem_malloc(int size);

extern void mem_free(void *data);

extern void *mem_calloc(int num, int size);

extern int util_tcp_send(int fd, const char *data, int len);

extern int util_tcp_recv(int fd, char *data, int len);

extern void util_set_fd_nonblocking(int fd);

extern int util_get_fd_ip_port(int fd,char *ip,int *port);

#endif
