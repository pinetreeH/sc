//
// Created by pinetree on 16-9-19.
//

#ifndef SC_HANDLER_HDL_H
#define SC_HANDLER_HDL_H

extern int hdl_recv_data(int fd, const char *data, int len);

extern int hdl_recv_close(int fd);

extern int hdl_recv_err(int fd);

#endif
