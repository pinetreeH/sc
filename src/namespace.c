//
// Created by pinetree on 16-10-29.
//

#include "namespace.h"
#include "room.h"
#include "hashmap.h"
#include "util.h"
#include "helper.h"
#include <stdlib.h>
#include <string.h>

#define NAMESPACE_NAME_STR_MAX 128

struct namespace {
    hashmap *rooms;
};

struct namespace *nsp_new(void) {
    struct namespace *nsp = NULL;
    nsp = (struct namespace *) mem_malloc(sizeof(struct namespace));
    if (nsp) {
        nsp->rooms = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                  str_cmp, pointer_cmp,
                                  NULL, room_del,
                                  str_hashindex);
    }
    return nsp;
}

int nsp_add_room(struct namespace *nsp, struct room *r) {
    if (nsp && r)
        return hashmap_set(nsp->rooms, (void *) room_name(r), (void *) r);
    return -1;
}

int nsp_del_room(struct namespace *nsp, struct room *r) {
    if (nsp && r)
        return hashmap_delete(nsp->rooms, (void *) room_name(r), 0, 1);
    return -1;
}

int nsp_del(struct namespace *nsp) {
    if (nsp)
        hashmap_free(nsp->rooms, 0, 1);
    return 0;
}