
//
// Created by pinetree on 10/1/16.
//

#include "room.h"
#include "hashmap.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

// room_name(str) -> room_detail(hashmap) key->client, value->NULL
static hashmap *rooms;

int room_init(int capacity) {
    if (rooms)
        hashmap_free(rooms, 0, 0);

    rooms = hashmap_init(capacity, hashmap_strkey_cmp,
                         NULL, NULL, NULL,
                         hashmap_strkey_hashindex);
}

int room_jion(const char *room_name, struct client *c) {
    if (!rooms || !room_name || !c)
        return -1;

    hashmap *one_room = NULL;
    int ret = hashmap_get(rooms, (void *) room_name, (void **) &one_room);
    if (ret == HASHMAP_OK) {
        // room exist
        hashmap_set(one_room, (void *) c, NULL);
    } else if (ret == HASHMAP_ELEMENT_NOT_FOUND) {
        // create a new room
        one_room = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                hashmap_pointerkey_cmp,
                                NULL, NULL, NULL,
                                hashmap_pointerkey_hashindex);
        if (!one_room)
            return -1;
        int len = strlen(room_name);
        char *room_name_str = (char *) mem_malloc(len + 1);
        strcpy(room_name_str, room_name);
        room_name_str[len] = '\0';
        hashmap_set(rooms, (void *) room_name_str, (void *) one_room);
        hashmap_set(one_room, (void *) c, NULL);
    } else {
        log_err("room_jion err!\n");
        return -1;
    }

    return 0;
}

int room_leave(const char *room_name, struct client *c) {
    if (!rooms || !room_name || !c)
        return -1;

    // delete client then delete room if room.size ==0
    hashmap *one_room = NULL;
    int ret = hashmap_get(rooms, (void *) room_name, (void **) &one_room);
    if (ret == HASHMAP_OK) {
        hashmap_delete(one_room, (void *) c, NULL, NULL);
        if (hashmap_size(one_room) == 0) {
            // TODO free string key function
            hashmap_free(one_room, 0, 0);
        }
    }
    return 0;
}






