//
// Created by pinetree on 16-10-29.
//

#ifndef SC_SESSION_SES_H
#define SC_SESSION_SES_H

struct reactor_base;
struct client;
struct session;
struct session_iterator;

extern struct session *ses_init(int capacity);

extern void ses_del(struct session *s);

extern int ses_add_client(struct session *s, struct client *c);

extern int ses_del_client_by_fd(struct session *s, int fd);

extern struct client *ses_get_client_by_fd(struct session *s, int fd);

extern struct client *ses_get_client_by_sid(struct session *s,
                                            const char *sid);

extern int ses_update_client_heartbeat(struct session *s,
                                       struct client *c,
                                       int new_heartbeat);

extern int ses_handle_timeout_client(struct reactor_base *ae,
                                     void *data);

extern int ses_is_new_connection(struct session *s, int fd,
                                 const char *data, int len);

// iterate all clients in session
extern struct session_iterator *ses_iterator_new(struct session *s);

extern int ses_iterator_del(struct session_iterator *it);

extern int ses_valid_iterator(struct session_iterator *it);

extern int ses_next_client(struct session *s,
                           struct session_iterator **it,
                           struct client **c);

#endif
