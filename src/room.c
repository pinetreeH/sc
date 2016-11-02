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

struct room_iterator {
    hashmap_iterator it;
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
        return hashmap_delete(r->clients, (void *) c);
    }
    return -1;
}

int room_del(void *room) {
    struct room *r = (struct room *) room;
    if (r) {
        hashmap_free(r->clients);
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

// wrapper for hashmap_iterator
struct room_iterator *room_iterator_new(struct room *r) {
    if (!r)
        return NULL;
    if (hashmap_size(r->clients) == 0)
        return NULL;

    struct room_iterator *room_it = NULL;
    room_it = (struct room_iterator *)
            mem_malloc(sizeof(struct room_iterator));
    room_it->it = hashmap_get_iterator(r->clients);
    return room_it;
}

int room_iterator_del(struct room_iterator *it) {
    mem_free(it);
    return 0;
}

int room_valid_iterator(struct room_iterator *it) {
    return it ? hashmap_valid_iterator(it->it) : 0;
}

int room_next_client(struct room *r,
                     struct room_iterator **it,
                     struct client **c) {
    if (!r || !it || !c)
        return -1;

    (*it)->it = hashmap_next((*it)->it, (void **) c, NULL);
    return 0;
}
