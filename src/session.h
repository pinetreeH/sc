//
// Created by pinetree on 16-9-10.
//

#ifndef SC_SESSION_SES_H
#define SC_SESSION_SES_H

//#define ROOM_NAME_MAX 32

struct client;

extern int ses_init(int capacity);

extern int ses_add_new_client(int fd, const char *sid);

extern int ses_del_client_by_fd(int fd);

extern int ses_del_client_by_sid(const char *sid);

extern int ses_get_min_time(void);

extern int ses_new_connection(int fd, const char *data, int data_len);

extern int ses_update_client_heartbeat_by_fd(int fd);

extern struct client **ses_get_clients(int *size);


#endif
