//
// Created by pinetree on 16-9-8.
//

#ifndef SC_REACTOR_AE_H
#define SC_REACTOR_AE_H

#include <sys/epoll.h>

#define AE_OK 0
#define AE_ERR -1

#define AE_EVENT_NONE 0
#define AE_NET_EVENT_READ 1
#define AE_NET_EVENT_WRITE 2
#define AE_NET_EVENT 3
#define AE_TIME_EVENT 4
#define AE_ALL_EVNET (AE_NET_EVENT|AE_TIME_EVENT)

struct reactor_base;

typedef void ae_process_fn(struct reactor_base *eventLoop,
                           int fd, void *fn_parameter, int mask);

extern struct reactor_base *ae_init(int capacity);

extern void ae_del(struct reactor_base *base);

extern int ae_add_net_event(struct reactor_base *base, int fd, int mask,
                            ae_process_fn *fn, void *fn_parameter,
                            const char *fn_name);

extern int ae_del_net_event(struct reactor_base *base, int fd,
                            int mask);

extern int ae_get_capacity(struct reactor_base *reactor_base);

extern void ae_stop(struct reactor_base *reactor_base);

extern void ae_run(struct reactor_base *eventLoop);

#endif
