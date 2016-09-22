//
// Created by pinetree on 16-9-9.
//

#ifndef UTIL_H
#define UTIL_H

#define NOTUSED_PARAMETER(p) ((void) p)

extern void log_debug(const char *fmt, ...);

extern void logInfo(const char *fmt, ...);

extern void log_error(const char *fmt, ...);

extern int get_value_of_key(const char *str, const char *key,
                            const char end_flag, char *value);

extern int get_timestamp(void);

extern int make_sid(char *sid);

extern void *mem_malloc(int size);

extern void mem_free(void *data);

extern void *mem_calloc(int num, int size);

extern int tcp_send(int fd, const char *data, int len);

extern int tcp_recv(int fd, char *data, int len);

extern void set_fd_nonblocking(int fd);

extern int get_msg_from_websocket_data(const char *websocket_data, int len,
                                       char *dst);

#endif
