//
// Created by pinetree on 16-9-24.
//

#ifndef SC_HANDLER_HDLIF_H
#define SC_HANDLER_HDLIF_H

struct handler_if {
    void (*on_connect)(int fd, const char *data, int len);

    void (*on_disconnect)(int fd, const char *data, int len);

    void (*on_event)(int fd, const char *data, int len);

    void (*on_ack)(int fd, const char *data, int len);

    void (*on_error)(int fd, const char *data, int len);

    void (*on_binary_event)(int fd, const char *data, int len);

    void (*on_binary_ack)(int fd, const char *data, int len);
};

#endif
