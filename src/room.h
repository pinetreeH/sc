//
// Created by pinetree on 16-10-29.
//

#ifndef SC_ROOM_ROOM_H
#define SC_ROOM_ROOM_H

struct client;

struct room;

extern struct room *room_new(void);

extern int room_jion(struct room *r, struct client *c);

extern int room_leave(struct room *r, struct client *c);

extern int room_del(void *room);

extern int room_client_number(struct room *r);

extern const char *room_get_name(struct room *r);

extern int room_set_name(struct room *r, const char *name);

#endif
