//
// Created by pinetree on 10/30/16.
//

#ifndef SC_SERVER_SRV_H
#define SC_SERVER_SRV_H

struct reactor_base;
struct session;
struct namespace;
struct handler_if;

struct server {
    struct reactor_base *ae;
    struct session *ses;
    struct namespace *nsp;
    int ping_interval;
    int ping_timeout;
    int port;
    int admin_port;
    struct handler_if *handler;
};

#endif
