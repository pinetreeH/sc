//
// Created by pinetree on 16-9-10.
//

#include "session.h"
#include "util.h"
#include <string.h>

#define CLIENT_SID_MAX 32

struct client {
    char sid[CLIENT_SID_MAX];
    int fd;
    int heartbeat;
    heap_element *heartbeat_in_sesssion;
};

struct fd_client {
    int fd;
    struct client *client;
};

struct session {
    int capacity;
    int size;
    struct fd_client *fd_to_client;// array
    hashmap *sid_to_client; // sid is a string
    heap *heartbeat;
};

static struct session sessions;

int ses_init(int capacity) {
    sessions.capacity = capacity;
    sessions.size = 0;
    sessions.sid_to_client = hashmap_init(capacity, hashmap_strkey_cmp,
                                          NULL, NULL, NULL,
                                          hashmap_strkey_hashindex);
    sessions.fd_to_client = mem_calloc(capacity, sizeof(struct fd_client));

    sessions.heartbeat = minheap_init(capacity);

    if (!sessions.sid_to_client)
        return -1;
    if (!sessions.fd_to_client) {
        mem_free(sessions.sid_to_client);
        return -1;
    }
    if (!sessions.heartbeat) {
        mem_free(sessions.sid_to_client);
        mem_free(sessions.fd_to_client);
        return -1;
    }

    return 0;
}

int ses_add_new_client(int fd, const char *sid) {
    if (fd >= sessions.capacity)
        return -1;

    struct client *c = mem_malloc(sizeof(struct client));
    if (!c)
        return -1;

    c->fd = fd;
    c->heartbeat = util_get_timestamp();
    strcpy(c->sid, sid);
    heap_element e;
    e.key = c->heartbeat;
    e.data = c;
    c->heartbeat_in_sesssion = minheap_insert(sessions.heartbeat, e);

    sessions.fd_to_client[fd].fd = fd;
    sessions.fd_to_client[fd].client = c;

    if (hashmap_set(sessions.sid_to_client, (void *) c->sid, (void *) c)
        == HASHMAP_OK)
        return 0;

    return -1;
}

int ses_del_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return -1;

    sessions.fd_to_client[fd].fd = 0;
    struct client *client = sessions.fd_to_client[fd].client;
    sessions.fd_to_client[fd].client = NULL;

    hashmap_delete(sessions.sid_to_client, (void *) client->sid, 0, 0);

    mem_free(client);
}

int ses_del_client_by_sid(const char *sid) {
    if (!sid)
        return -1;

    struct client *client = NULL;
    if (hashmap_get(sessions.sid_to_client, (void *) sid, client)
        == HASHMAP_OK) {
        return ses_del_client_by_fd(client->fd);
    }
    return -1;
}

struct client *ses_get_client_by_sid(const char *sid) {
    if (!sid || !sessions.sid_to_client)
        return NULL;

    struct client *client = NULL;
    hashmap_get(sessions.sid_to_client, (void *) sid, &client);
    return client;
}

struct client *ses_get_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return NULL;

    return sessions.fd_to_client[fd].client;
}

int ses_get_min_time(void) {
    heap_element e = minheap_get_top(sessions.heartbeat);
    if (e.key > 0)
        return e.key;

}

int ses_new_connection(int fd, const char *data, int data_len) {
    // parse data by protocol
    UTIL_NOTUSED(data);
    UTIL_NOTUSED(data_len);

    if (!ses_get_client_by_fd(fd))
        return 1;
    return 0;
}

int ses_update_client_heartbeat_by_fd(int fd) {
    struct client *c = sessions.fd_to_client[fd].client;
    if (!c)
        return 0;

    c->heartbeat = util_get_timestamp();
    c->heartbeat_in_sesssion = minheap_update(sessions.heartbeat,
                                              c->heartbeat_in_sesssion,
                                              c->heartbeat);
}