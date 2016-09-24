//
// Created by pinetree on 16-9-19.
//

#ifndef SC_HANDLER_HDL_H
#define SC_HANDLER_HDL_H

struct handler_if;

extern int hdl_recv_data(int fd, const char *data, int len);

extern int hdl_recv_close(int fd);

extern int hdl_recv_err(int fd);

extern int hdl_register_handler(const char *nsp, struct handler_if *h);

//extern int hdl_init(void);

#endif
