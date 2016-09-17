//
// Created by pinetree on 16-9-10.
//

#include "session.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>


static struct session sessions;

int session_init(int capacity) {
    sessions.capacity = capacity;
    sessions.size = 0;
    sessions.sid_to_client = hashmap_init(capacity, hashmap_strkey_cmp, NULL,
                                          NULL, NULL, hashmap_strkey_hashindex);
    sessions.fd_to_client = mem_calloc(capacity, sizeof(struct fd_client));

    if (!sessions.sid_to_client)
        return -1;
    if (!sessions.fd_to_client) {
        mem_free(sessions.sid_to_client);
        return -1;
    }

    return 0;
}

int add_new_client(int fd, const char *sid) {
    if (fd >= sessions.capacity)
        return -1;

    struct client *c = mem_malloc(sizeof(struct client));
    if (!c)
        return -1;

    c->fd = fd;
    c->heartbeat = get_timestamp();
    strcpy(c->sid, sid);

    sessions.fd_to_client[fd].fd = fd;
    sessions.fd_to_client->client = c;

    if (hashmap_set(sessions.sid_to_client, (void *) sid, (void *) c) == HASHMAP_OK)
        return 0;

    return -1;
}

int delete_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return -1;

    sessions.fd_to_client[fd].fd = 0;
    struct client *client = sessions.fd_to_client[fd].client;
    sessions.fd_to_client[fd].client = NULL;

    hashmap_delete(sessions.sid_to_client, (void *) client->sid, 0, 0);

    mem_free(client);
}

int delete_client_by_sid(const char *sid) {
    if (!sid)
        return -1;

    struct client *client = NULL;
    if (hashmap_get(sessions.sid_to_client, (void *) sid, client)
        == HASHMAP_OK) {
        return delete_client_by_fd(client->fd);
    }
    return -1;
}

struct client *get_client_by_sid(char *sid) {
    if (!sid || !sessions.sid_to_client)
        return NULL;

    hashmap_key key;
    key.void_key = (void *) sid;
    struct client *client = NULL;
    hashmap_get(sessions.sid_to_client, &key, &client);
    return client;
}

struct client *get_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return NULL;

    return sessions.fd_to_client[fd].client;
}

int get_min_time(void) {
    return 1000;
}