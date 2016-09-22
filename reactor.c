//
// Created by pinetree on 16-9-8.
//

#include "reactor.h"
#include "util.h"

struct reactor_net_event {
    int mask;
    const char *read_fn_name;// for debug
    const char *write_fn_name;
    ae_process_fn *read_fn;
    ae_process_fn *write_fn;
    void *fn_parameter;
};


struct reactor_base {
    int maxfd;
    int capacity;
    struct reactor_net_event *net_events;
    int stop;
    int epoll_fd;
    struct epoll_event *ep_events;
};


struct reactor_base *ae_init(int capacity) {
    struct reactor_base *base = mem_malloc(sizeof(struct reactor_base));
    do {
        if (!base) {
            break;
        }
        base->net_events = mem_malloc(sizeof(struct reactor_net_event) *
                                      capacity);
        if (!base->net_events) {
            mem_free(base);
            break;
        }
        base->maxfd = -1;
        base->capacity = capacity;
        base->stop = 0;
        base->epoll_fd = epoll_create(1024);
        if (base->epoll_fd <= 0) {
            mem_free(base->net_events);
            mem_free(base);
            break;
        }
        base->ep_events = mem_malloc(sizeof(struct epoll_event) * capacity);
        if (!base->ep_events) {
            mem_free(base->net_events);
            mem_free(base);
            break;
        }
    } while (0);
    return base;
}

void ae_del(struct reactor_base *base) {
    if (!base)
        return;
    mem_free(base->net_events);
    mem_free(base->ep_events);
    mem_free(base);
}

void ae_stop(struct reactor_base *base) {
    base->stop = 1;
}

int ae_add_net_event(struct reactor_base *base, int fd, int mask,
                     ae_process_fn *fn, void *fn_parameter,
                     const char *fn_name) {
    int ret = AE_ERR;
    do {
        if (fd >= base->capacity) {
            break;
        }
        struct epoll_event ep = {0};
        ep.events = 0;
        struct reactor_net_event *net_event = &base->net_events[fd];
        int old_mask = net_event->mask;
        int new_mask = mask | old_mask;
        if (new_mask & AE_EVENT_READ) {
            ep.events |= EPOLLIN;
        }
        if (new_mask & AE_EVENT_WRITE) {
            ep.events |= EPOLLOUT;
        }
        ep.data.fd = fd;
        int op = (old_mask == AE_EVENT_NONE ? EPOLL_CTL_ADD :
                  EPOLL_CTL_MOD);
        if (epoll_ctl(base->epoll_fd, op, fd, &ep) == -1) {
            break;
        }
        net_event->mask = new_mask;
        if (mask & AE_EVENT_READ) {
            net_event->read_fn = fn;
            net_event->read_fn_name = fn_name;
        }
        if (mask & AE_EVENT_WRITE) {
            net_event->write_fn = fn;
            net_event->write_fn_name = fn_name;
        }
        net_event->fn_parameter = fn_parameter;
        if (fd > base->maxfd) {
            base->maxfd = fd;
        }
        ret = AE_OK;
    } while (0);
    return ret;
}

int ae_del_net_event(struct reactor_base *base, int fd, int mask) {
    int ret = AE_ERR;
    do {
        if (fd >= base->capacity) {
            break;
        }
        struct reactor_net_event *fe = &base->net_events[fd];
        int old_mask = fe->mask;
        if (old_mask == AE_EVENT_NONE) {
            ret = AE_OK;
            break;
        }
        struct epoll_event ep = {0};
        ep.events = 0;
        int new_mask = old_mask & (~mask);
        if (new_mask & AE_EVENT_READ) {
            ep.events |= EPOLLIN;
        }
        if (new_mask & AE_EVENT_WRITE) {
            ep.events |= EPOLLOUT;
        }
        ep.data.fd = fd;
        int op = (new_mask == AE_EVENT_NONE ? EPOLL_CTL_DEL :
                  EPOLL_CTL_MOD);
        if (epoll_ctl(base->epoll_fd, op, fd, &ep) == -1) {
            break;
        }
        fe->mask = new_mask;
//        if (fe->mask == REACTOR_EVENT_NONE) {
//            fe->read_fn = fe->write_fn = NULL;
//            if (fe->fn_parameter)free(fe->fn_parameter);
//        }
        if (fd == base->maxfd && fe->mask == AE_EVENT_NONE) {
            for (int i = base->maxfd - 1; i >= 0; i--) {
                if (base->net_events[i].mask != AE_EVENT_NONE) {
                    break;
                }
                base->maxfd = i;//maxfd maybe 0
            }
        }
    } while (0);
    return ret;
}

int ae_get_capacity(struct reactor_base *base) {
    return base->capacity;
}

static int reactor_process_net_event(struct reactor_base *base) {
    int processed_cnt = 0;
    //TODO
    int min_time = ses_get_min_time();
    int fds = epoll_wait(base->epoll_fd, base->ep_events, base->capacity, min_time);
    for (int i = 0; i < fds; i++) {
        struct epoll_event *ep = base->ep_events + i;
        int mask = 0;
        if (ep->events & EPOLLIN) {
            mask |= AE_EVENT_READ;
        }
        if (ep->events & EPOLLOUT) {
            mask |= AE_EVENT_WRITE;
        }
        if (ep->events & EPOLLERR) {
            mask |= AE_EVENT_WRITE;
        }
        if (ep->events & EPOLLHUP) {
            mask |= AE_EVENT_WRITE;
        }
        int fd = ep->data.fd;
        struct reactor_net_event *net_event = base->net_events + fd;
        int read_fn_called = 0;
        if (net_event->mask & mask & AE_EVENT_READ) {
            read_fn_called = 1;
            log_debug("read_fn_name:%s\n", net_event->read_fn_name);
            net_event->read_fn(base, fd, net_event->fn_parameter, mask);
        }
        if (net_event->mask & mask & AE_EVENT_WRITE) {
            if (!read_fn_called || net_event->write_fn != net_event->read_fn) {
                log_debug("write_fn_name:%s\n", net_event->read_fn_name);
                net_event->write_fn(base, fd, net_event->fn_parameter, mask);
            }
        }
        processed_cnt++;
    }
    return processed_cnt;
}

void ae_run(struct reactor_base *base) {
    if (!base) {
        return;
    }
    while (!base->stop) {
        reactor_process_net_event(base);
        // process timeout event
    }
}