//
// Created by pinetree on 16-10-29.
//

#include "client_manager.h"
#include "client.h"
#include "hashmap.h"
#include "heap.h"
#include "helper.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

struct client_manager {
    int capacity;
    struct client **fd_to_clients; // array, fd as index
    hashmap *sid_to_client;
    heap *heartbeat;
};

struct client_manager *cmg_init(int capacity) {
    struct client_manager *cmg = NULL;
    cmg = (struct client *) mem_malloc(sizeof(struct client_manager));
    if (cmg) {
        cmg->capacity = capacity;
        cmg->fd_to_clients = (struct client **)
                mem_calloc(capacity, sizeof(struct client *));
        cmg->sid_to_client = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                          str_cmp, pointer_cmp,
                                          NULL, client_del,
                                          str_hashindex);
        cmg->heartbeat = heap_init(capacity, minheap_key_cmp);
    }
    return cmg;
}

int cmg_add_client(struct client_manager *cmg, struct client *c) {
    if (cmg && c) {
        cmg->fd_to_clients[client_get_fd(c)] = c;
        hashmap_set(cmg->sid_to_client, (void *) client_get_sid(c),
                    (void *) c);
        client_set_hearbeat_ptr(c, heap_insert(cmg->heartbeat,
                                               (void *) client_get_heartbeat(c),
                                               (void *) c));
    }
    return 0;
}

int cmg_del_client_by_fd(struct client_manager *cmg, int fd) {
    if (cmg && fd) {
        struct client *c = cmg->fd_to_clients[fd];
        cmg->fd_to_clients[fd] = NULL;
        hashmap_delete(cmg->sid_to_client, (void *) client_get_sid(c), 0, 1);
        minheap_element_del(cmg->heartbeat, client_get_hearbeat_ptr(c));
    }
    return 0;
}

struct client *cmg_get_client_by_fd(struct client_manager *cmg, int fd) {
    return cmg && fd ? cmg->fd_to_clients[fd] : NULL;
}

struct client *cmg_get_client_by_sid(struct client_manager *cmg,
                                     const char *sid) {
    if (cmg && sid) {
        struct client *c = NULL;
        hashmap_get(cmg->sid_to_client, (void *) sid, (void **) &c);
        return c;
    }
    return NULL;
}

int cmg_update_client_heartbeat_by_fd(struct client_manager *cmg,
                                      int fd, int new_heartbeat) {
    if (cmg && fd) {
        struct client *c = NULL;
        c = cmg->fd_to_clients[fd];
        client_set_heartbeat(c, new_heartbeat);
        client_set_hearbeat_ptr(c, minheap_update(cmg->heartbeat,
                                                  client_get_hearbeat_ptr(c),
                                                  (void *) new_heartbeat,
                                                  minheap_key_update));
    }
    return 0;
}

int cmg_handle_timeout_client(struct client_manager *cmg,
                              struct reactor_base *ae,
                              void *heartbeat_timeout) {
    UTIL_NOTUSED(ae);
    if (!cmg)
        return 0;
    if (!heap_size(cmg->heartbeat))
        return 0;

    int current_timestamp = util_get_timestamp();
    int *k = NULL;
    struct client *c = NULL;
    do {
        heap_get_root(cmg->heartbeat, (void **) &k, (void **) &c);
        if (!k || !c)
            break;
        int last_timestamp = (int) k;
        int timeout = (int) heartbeat_timeout;
        if (current_timestamp - last_timestamp < timeout)
            break;
        cmg_del_client_by_fd(cmg, client_get_fd(c));
    } while (1);

    return 0;
}