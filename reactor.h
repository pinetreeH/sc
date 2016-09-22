//
// Created by pinetree on 16-9-8.
//

#ifndef REACTOR_H
#define REACTOR_H

#include <sys/epoll.h>

#define REACTOR_OK 0
#define REACTOR_ERR -1

#define REACTOR_EVENT_NONE 0
#define REACTOR_EVENT_READ 1
#define REACTOR_EVENT_WRITE 2

#define REACTOR_NET_EVENT 1
#define REACTOR_TIME_EVENT 2
#define REACTOR_ALL_EVNET (REACTOR_NET_EVENT|REACTOR_TIME_EVENT)


struct reactor_base;

typedef void process_fn(struct reactor_base *eventLoop,
                        int fd, void *fn_parameter, int mask);

struct reactor_base *reactor_base_init(int capacity);

void reactor_base_delete(struct reactor_base *base);

int reactor_add_net_event(struct reactor_base *base, int fd, int mask,
                          process_fn *fn, void *fn_parameter,
                          const char *fn_name);

int reactor_delete_net_event(struct reactor_base *base, int fd,
                             int mask);

int reactor_get_capacity(struct reactor_base *reactor_base);

void reactor_stop(struct reactor_base *reactor_base);

void reactor_run(struct reactor_base *eventLoop);

#endif
