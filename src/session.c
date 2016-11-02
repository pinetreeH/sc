//
// Created by pinetree on 16-10-29.
//

#include "session.h"
#include "client.h"
#include "hashmap.h"
#include "heap.h"
#include "helper.h"
#include "util.h"
#include "server.h"
#include <stdlib.h>
#include <string.h>

struct session {
    int capacity;
    struct client **fd_to_clients; // array, fd as index
    hashmap *sid_to_client;
    heap *heartbeat;
};

struct session_iterator {
    hashmap_iterator it;
};

struct session *ses_init(int capacity) {
    struct session *s = NULL;
    s = (struct session *) mem_malloc(sizeof(struct session));
    if (s) {
        s->capacity = capacity;
        s->fd_to_clients = (struct client **)
                mem_calloc(capacity, sizeof(struct client *));
        s->sid_to_client = hashmap_init(HASHMAP_DEFAULT_CAPACITY,
                                        str_cmp, pointer_cmp,
                                        NULL, client_del,
                                        str_hashindex);
        s->heartbeat = heap_init(capacity, minheap_key_cmp);
    }
    return s;
}

void ses_del(struct session *s) {
    if (!s)
        return;
    mem_free(s->fd_to_clients);
    hashmap_free(s->sid_to_client);
    heap_del(s->heartbeat);
}

int ses_add_client(struct session *s, struct client *c) {
    if (s && c) {
        s->fd_to_clients[client_fd(c)] = c;
        hashmap_set(s->sid_to_client, (void *) client_sid(c),
                    (void *) c);
        client_set_hearbeat_ptr(c, heap_insert(s->heartbeat,
                                               (void *) client_heartbeat(c),
                                               (void *) c));
    }
    return 0;
}

int ses_del_client_by_fd(struct session *s, int fd) {
    if (s && fd) {
        struct client *c = s->fd_to_clients[fd];
        s->fd_to_clients[fd] = NULL;
        hashmap_delete(s->sid_to_client, (void *) client_sid(c));
        minheap_element_del(s->heartbeat, client_get_hearbeat_ptr(c));
    }
    return 0;
}

struct client *ses_get_client_by_fd(struct session *s, int fd) {
    return s && fd ? s->fd_to_clients[fd] : NULL;
}

struct client *ses_get_client_by_sid(struct session *s,
                                     const char *sid) {
    if (s && sid) {
        struct client *c = NULL;
        hashmap_get(s->sid_to_client, (void *) sid, (void **) &c);
        return c;
    }
    return NULL;
}

int ses_update_client_heartbeat(struct session *s,
                                struct client *c,
                                int new_heartbeat) {
    if (s && c) {
        client_set_heartbeat(c, new_heartbeat);
        client_set_hearbeat_ptr(c, minheap_update(s->heartbeat,
                                                  client_get_hearbeat_ptr(c),
                                                  (void *) new_heartbeat,
                                                  minheap_key_update));
    }
    return 0;
}

int ses_handle_timeout_client(struct reactor_base *ae, void *data) {
    UTIL_NOTUSED(ae);
    struct server *srv = (struct server *) data;
    if (!srv->ses)
        return 0;
    if (!heap_size(srv->ses->heartbeat))
        return 0;

    int current_timestamp = util_get_timestamp();
    int *k = NULL;
    struct client *c = NULL;
    do {
        heap_get_root(srv->ses->heartbeat, (void **) &k, (void **) &c);
        if (!k || !c)
            break;
        int last_timestamp = (int) k;
        if (current_timestamp - last_timestamp < srv->ping_timeout)
            break;
        ses_del_client_by_fd(srv->ses, client_fd(c));
    } while (1);

    return 0;
}

int ses_is_new_connection(struct session *s, int fd,
                          const char *data, int len) {
    return !s->fd_to_clients[fd] ? 1 : 0;
}

// wrapper for hashmap_iterator
struct session_iterator *ses_iterator_new(struct session *s) {
    if (!s)
        return NULL;
    if (hashmap_size(s->sid_to_client) == 0)
        return NULL;

    struct session_iterator *ses_it = NULL;
    ses_it = (struct session_iterator *)
            mem_malloc(sizeof(struct session_iterator));
    ses_it->it = hashmap_get_iterator(s->sid_to_client);
    return ses_it;
}

int ses_iterator_del(struct session_iterator *it) {
    mem_free(it);
    return 0;
}

int ses_valid_iterator(struct session_iterator *it) {
    return it ? hashmap_valid_iterator(it->it) : 0;
}

int ses_next_client(struct session *s,
                    struct session_iterator **it,
                    struct client **c) {
    if (!s || !it || !c)
        return -1;

    (*it)->it = hashmap_next((*it)->it, NULL, (void **) c);
    return 0;
}
