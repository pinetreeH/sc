//
// Created by pinetree on 10/1/16.
//

#ifndef SC_ROOM_ROOM_H
#define SC_ROOM_ROOM_H

struct client;

extern int room_join(const char *room_name, struct client *c);

extern int room_leave(const char *room_name, struct client *c);

extern int room_init(int capacity);

#endif
