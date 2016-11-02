//
// Created by pinetree on 16-10-29.
//

#include "nsp.h"
#include "room.h"
#include "hashmap.h"
#include "util.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>

#define NAMESPACE_NAME_STR_MAX 128

struct nsp {
    hashmap *rooms;
};

struct nsp_iterator {
    hashmap_iterator it;
};

struct nsp *nsp_new(void) {
    struct nsp *nsp = NULL;
    nsp = (struct nsp *) mem_malloc(sizeof(struct nsp));
    if (nsp) {
        nsp->rooms = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                  str_cmp, pointer_cmp,
                                  NULL, room_del,
                                  str_hashindex);
    }
    return nsp;
}

struct room *nsp_get_room(struct nsp *n, const char *room_name) {
    if (!n || !room_name)
        return NULL;
    struct room *r = NULL;
    hashmap_get(n->rooms, (void *) room_name, (void **) &r);
    return r;
}

int nsp_add_room(struct nsp *n, struct room *r) {
    if (n && r)
        return hashmap_set(n->rooms, (void *) room_name(r), (void *) r);
    return -1;
}

int nsp_del_room(struct nsp *n, struct room *r) {
    if (n && r)
        return hashmap_delete(n->rooms, (void *) room_name(r));
    return -1;
}

int nsp_del(struct nsp *n) {
    if (n)
        hashmap_free(n->rooms);
    return 0;
}

// wrapper for hashmap_iterator
struct nsp_iterator *nsp_iterator_new(struct nsp *n) {
    if (!n)
        return NULL;
    if (hashmap_size(n->rooms) == 0)
        return NULL;

    struct nsp_iterator *it = NULL;
    it = (struct nsp_iterator *)
            mem_malloc(sizeof(struct nsp_iterator));
    it->it = hashmap_get_iterator(n->rooms);
    return it;
}

int nsp_iterator_del(struct nsp_iterator *it) {
    mem_free(it);
    return 0;
}

int nsp_valid_iterator(struct nsp_iterator *it) {
    return it ? hashmap_valid_iterator(it->it) : 0;
}

int nsp_next_room(struct nsp *n,
                  struct nsp_iterator **it,
                  struct room **r) {
    if (!n || !it || !r)
        return -1;

    (*it)->it = hashmap_next((*it)->it, NULL, (void **) r);
    return 0;
}