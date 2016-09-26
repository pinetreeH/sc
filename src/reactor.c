//
// Created by pinetree on 16-9-8.
//

#include "reactor.h"
#include "minheap.h"
#include "util.h"
#include <stdlib.h>
#include <sys/epoll.h>

#define DEFAULT_WAIT_TIME 1000

struct net_event {
    int mask;
    const char *read_fn_name;// for debug
    const char *write_fn_name;
    ae_net_process_fn *read_fn;
    ae_net_process_fn *write_fn;
    void *fn_parameter;
};

struct time_event {
    int last_proc_timestamp;
    int repeat;
    int interval;
    const char *fn_name;
    void *fn_parameter;
    ae_time_process_fn *fn;
};

struct reactor_base {
    int maxfd;
    int capacity;
    struct net_event *net_events;
    int stop;
    int epoll_fd;
    struct epoll_event *ep_events;
    heap *time_events;
};

static int process_net_event(struct reactor_base *ae);

static int process_time_event(struct reactor_base *ae);

static int get_min_wait_time(struct reactor_base *ae);


struct reactor_base *ae_init(int capacity) {
    struct reactor_base *base = mem_malloc(sizeof(struct reactor_base));
    do {
        if (!base) {
            break;
        }
        base->net_events = mem_malloc(sizeof(struct net_event) *
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
        base->time_events = heap_init(10, minheap_key_cmp);
        if (!base->time_events) {
            mem_free(base->net_events);
            mem_free(base);
        }
    } while (0);
    return base;
}

void ae_del(struct reactor_base *ae) {
    if (!ae)
        return;
    mem_free(ae->net_events);
    mem_free(ae->ep_events);
    heap_del(ae->time_events);
    mem_free(ae);
}

void ae_stop(struct reactor_base *ae) {
    ae->stop = 1;
}

int ae_add_net_event(struct reactor_base *ae, int fd, int mask,
                     ae_net_process_fn *fn, void *fn_parameter,
                     const char *fn_name) {
    int ret = AE_ERR;
    do {
        if (fd >= ae->capacity) {
            break;
        }
        struct epoll_event ep = {0};
        ep.events = 0;
        struct net_event *net_event = &ae->net_events[fd];
        int old_mask = net_event->mask;
        int new_mask = mask | old_mask;
        if (new_mask & AE_NET_EVENT_READ) {
            ep.events |= EPOLLIN;
        }
        if (new_mask & AE_NET_EVENT_WRITE) {
            ep.events |= EPOLLOUT;
        }
        ep.data.fd = fd;
        int op = (old_mask == AE_EVENT_NONE ? EPOLL_CTL_ADD :
                  EPOLL_CTL_MOD);
        if (epoll_ctl(ae->epoll_fd, op, fd, &ep) == -1) {
            break;
        }
        net_event->mask = new_mask;
        if (mask & AE_NET_EVENT_READ) {
            net_event->read_fn = fn;
            net_event->read_fn_name = fn_name;
        }
        if (mask & AE_NET_EVENT_WRITE) {
            net_event->write_fn = fn;
            net_event->write_fn_name = fn_name;
        }
        net_event->fn_parameter = fn_parameter;
        if (fd > ae->maxfd) {
            ae->maxfd = fd;
        }
        ret = AE_OK;
    } while (0);
    return ret;
}

int ae_del_net_event(struct reactor_base *ae, int fd, int mask) {
    int ret = AE_ERR;
    do {
        if (fd >= ae->capacity) {
            break;
        }
        struct net_event *fe = &ae->net_events[fd];
        int old_mask = fe->mask;
        if (old_mask == AE_EVENT_NONE) {
            ret = AE_OK;
            break;
        }
        struct epoll_event ep = {0};
        ep.events = 0;
        int new_mask = old_mask & (~mask);
        if (new_mask & AE_NET_EVENT_READ) {
            ep.events |= EPOLLIN;
        }
        if (new_mask & AE_NET_EVENT_WRITE) {
            ep.events |= EPOLLOUT;
        }
        ep.data.fd = fd;
        int op = (new_mask == AE_EVENT_NONE ? EPOLL_CTL_DEL :
                  EPOLL_CTL_MOD);
        if (epoll_ctl(ae->epoll_fd, op, fd, &ep) == -1) {
            break;
        }
        fe->mask = new_mask;
//        if (fe->mask == REACTOR_EVENT_NONE) {
//            fe->read_fn = fe->write_fn = NULL;
//            if (fe->fn_parameter)free(fe->fn_parameter);
//        }
        if (fd == ae->maxfd && fe->mask == AE_EVENT_NONE) {
            for (int i = ae->maxfd - 1; i >= 0; i--) {
                if (ae->net_events[i].mask != AE_EVENT_NONE) {
                    break;
                }
                ae->maxfd = i;//maxfd maybe 0
            }
        }
    } while (0);
    return ret;
}

int ae_get_capacity(struct reactor_base *ae) {
    return ae->capacity;
}

int ae_add_time_event(struct reactor_base *ae,
                      ae_time_process_fn *fn, void *fn_parameter,
                      const char *fn_name, int interval, int repeat) {
    if (!ae || !fn || repeat < AE_TIME_EVENT_REPEAT_INFINITE)
        return -1;

    struct time_event *e = (struct time_event *)
            mem_malloc(sizeof(struct time_event));
    e->last_proc_timestamp = util_get_timestamp();
    e->fn = fn;
    e->fn_name = fn_name;
    e->repeat = repeat;
    e->interval = interval;
    int k = e->last_proc_timestamp + e->interval;
    heap_insert(ae->time_events, (void *) &k, (void *) e);

    return 0;
}

void ae_run(struct reactor_base *ae, int mask) {
    if (!ae)
        return;

    while (!ae->stop) {
        if (mask & AE_NET_EVENT)
            process_net_event(ae);
        if (mask & AE_TIME_EVENT)
            process_time_event(ae);
    }
}

static int process_net_event(struct reactor_base *ae) {
    int processed_cnt = 0;
    int min_time = get_min_wait_time(ae);
    int fds = epoll_wait(ae->epoll_fd, ae->ep_events, ae->capacity, min_time);
    for (int i = 0; i < fds; i++) {
        struct epoll_event *ep = ae->ep_events + i;
        int mask = 0;
        if (ep->events & EPOLLIN) {
            mask |= AE_NET_EVENT_READ;
        }
        if (ep->events & EPOLLOUT) {
            mask |= AE_NET_EVENT_WRITE;
        }
        if (ep->events & EPOLLERR) {
            mask |= AE_NET_EVENT_WRITE;
        }
        if (ep->events & EPOLLHUP) {
            mask |= AE_NET_EVENT_WRITE;
        }
        int fd = ep->data.fd;
        struct net_event *net_event = ae->net_events + fd;
        int read_fn_called = 0;
        if (net_event->mask & mask & AE_NET_EVENT_READ) {
            read_fn_called = 1;
            log_debug("read_fn_name:%s\n", net_event->read_fn_name);
            net_event->read_fn(ae, fd, net_event->fn_parameter, mask);
        }
        if (net_event->mask & mask & AE_NET_EVENT_WRITE) {
            if (!read_fn_called || net_event->write_fn != net_event->read_fn) {
                log_debug("write_fn_name:%s\n", net_event->read_fn_name);
                net_event->write_fn(ae, fd, net_event->fn_parameter, mask);
            }
        }
        processed_cnt++;
    }
    return processed_cnt;
}

static int get_min_wait_time(struct reactor_base *ae) {
    int time = DEFAULT_WAIT_TIME;
    do {
        if (heap_size(ae->time_events) == 0)
            break;

        struct time_event *e = NULL;
        int k = 0;
        int tmp = heap_get_root(ae->time_events, (void **) &k,
                                (void **) &e);
        if (tmp == -1) {
            log_err("get_min_wait_time err!\n");
            break;
        }

        int tmp_interval = util_get_timestamp() - e->last_proc_timestamp;
        if (tmp_interval <= e->interval)
            time = (DEFAULT_WAIT_TIME < tmp_interval ? DEFAULT_WAIT_TIME :
                    tmp_interval);
        else
            time = 0;
    } while (0);

    return time;
}

static int process_time_event(struct reactor_base *ae) {
    if (!ae)
        return -1;
    if (!ae->time_events || heap_size(ae->time_events) == 0)
        return 0;

    int current_timestamp = util_get_timestamp();
    int k = 0;
    struct time_event *e = NULL;
    do {
        heap_get_root(ae->time_events, (void **) &k, (void **) &e);
        if (k < current_timestamp)
            break;

        e->fn(ae->time_events, e->fn_parameter);
        if (e->repeat != AE_TIME_EVENT_REPEAT_INFINITE)
            e->repeat--;

        if (e->repeat == 0)
            heap_del_root(ae->time_events);
        else {
            e->last_proc_timestamp = current_timestamp;
            int new_k = e->last_proc_timestamp + e->interval;
            minheap_update(ae->time_events, heap_root_pos(ae->time_events),
                           (void *) &new_k, minheap_key_update);
        }
    } while (1);
    return 0;
}



