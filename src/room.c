//
// Created by pinetree on 16-10-29.
//

#include "room.h"
#include "util.h"
#include "helper.h"
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

#define ROOM_NAME_STR_MAX 128

struct room {
    char name[ROOM_NAME_STR_MAX];
    hashmap *clients;
};

struct room *room_new(const char *name) {
    struct room *r = NULL;
    r = (struct room *) mem_malloc(sizeof(struct room));
    if (r) {
        r->clients = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                  pointer_cmp, NULL,
                                  NULL, NULL,
                                  pointer_hashindex);
        strcpy(r->name, name);
    }
    return r;
}

int room_jion(struct room *r, struct client *c) {
    if (r && c) {
        return hashmap_set(r->clients, (void *) c, NULL);
    }
    return -1;
}

int room_leave(struct room *r, struct client *c) {
    if (r && c) {
        return hashmap_delete(r->clients, (void *) c, 0, 0);
    }
    return -1;
}

int room_del(void *room) {
    struct room *r = (struct room *) room;
    if (r) {
        hashmap_free(r->clients, 0, 0);
        mem_free(r);
    }
    return 0;
}

int room_client_number(struct room *r) {
    return r ? hashmap_size(r->clients) : -1;
}

const char *room_name(struct room *r) {
    return r ? r->name : NULL;
}
