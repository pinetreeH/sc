//
// Created by pinetree on 16-9-8.
//

#ifndef SC_REACTOR_AE_H
#define SC_REACTOR_AE_H

#define AE_OK 0
#define AE_ERR -1

#define AE_EVENT_NONE (1<<0)
#define AE_NET_EVENT_READ (1<<1)
#define AE_NET_EVENT_WRITE (1<<2)
#define AE_NET_EVENT (1<<3)
#define AE_TIME_EVENT (1<<4)
#define AE_TIME_EVENT_REPEAT_INFINITE  -1

struct reactor_base;

typedef void ae_net_process_fn(struct reactor_base *ae,
                               int fd, void *fn_parameter, int mask);

typedef void ae_time_process_fn(struct reactor_base *ae, void *fn_parameter);

extern struct reactor_base *ae_init(int capacity);

extern void ae_del(struct reactor_base *ae);

extern int ae_add_net_event(struct reactor_base *ae, int fd, int mask,
                            ae_net_process_fn *fn, void *fn_parameter,
                            const char *fn_name);

extern int ae_del_net_event(struct reactor_base *ae, int fd,
                            int mask);

extern int ae_get_capacity(struct reactor_base *ae);

extern void ae_stop(struct reactor_base *ae);

extern void ae_run(struct reactor_base *ae, int mask);

extern int ae_add_time_event(struct reactor_base *ae,
                             ae_time_process_fn *fn, void *fn_parameter,
                             const char *fn_name, int interval, int repeat);

#endif
