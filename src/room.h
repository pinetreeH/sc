//
// Created by pinetree on 16-10-29.
//

#ifndef SC_ROOM_ROOM_H
#define SC_ROOM_ROOM_H

#define ROOM_NAME_STR_MAX 32

struct client;
struct room;
struct room_iterator;

extern struct room *room_new(const char *name);

extern int room_jion(struct room *r, struct client *c);

extern int room_leave(struct room *r, struct client *c);

extern int room_del(void *room);

extern int room_client_number(struct room *r);

extern const char *room_name(struct room *r);

// iterator all clients in room
extern struct room_iterator *room_iterator_new(struct room *r);

extern int room_iterator_del(struct room_iterator *it);

extern int room_valid_iterator(struct room_iterator *it);

extern int room_next_client(struct room *r,
                            struct room_iterator **it,
                            struct client **c);


#endif
