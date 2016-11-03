//
// Created by pinetree on 16-9-24.
//

#ifndef SC_HANDLER_HDLIF_H
#define SC_HANDLER_HDLIF_H

struct server;

struct handler_if {
    void (*on_connect)(struct server *srv, int fd, const char *data, int len);

    void (*on_disconnect)(struct server *srv, int fd, const char *data, int len);

    void (*on_event)(struct server *srv, int fd, const char *data, int len);

    void (*on_ack)(struct server *srv, int fd, const char *data, int len);

    void (*on_error)(struct server *srv, int fd, const char *data, int len);

    void (*on_binary_event)(struct server *srv, int fd, const char *data, int len);

    void (*on_binary_ack)(struct server *srv, int fd, const char *data, int len);
};

#endif
