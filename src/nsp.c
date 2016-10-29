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
    char name[NAMESPACE_NAME_STR_MAX];
    hashmap *rooms;
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

int nsp_add_room(struct nsp *nsp, struct room *r) {
    if (nsp && r)
        return hashmap_set(nsp->rooms, (void *) room_get_name(r), (void *) r);
    return -1;
}

int nsp_del_room(struct nsp *nsp, struct room *r) {
    if (nsp && r)
        return hashmap_delete(nsp->rooms, (void *) room_get_name(r), 0, 1);
    return -1;
}

int nsp_del(struct nsp *nsp) {
    if (nsp)
        hashmap_free(nsp->rooms, 0, 1);
    return 0;
}

int nsp_set_name(struct nsp *nsp, const char *name) {
    if (nsp && name) {
        strcpy(nsp->name, name);
        return 0;
    }
    return -1;
}

const char *nsp_get_name(struct nsp *nsp) {
    return nsp ? nsp->name : NULL;
}