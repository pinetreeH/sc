//
// Created by pinetree on 16-9-10.
//

#include "session.h"
#include "client.h"
#include "util.h"
#include "hashmap.h"
#include "heap.h"
#include <string.h>
#include <arpa/inet.h>

struct session {
    int capacity;
    int size;
    struct client **clients;// array
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
    sessions.clients = (struct client **)
            mem_calloc(capacity, sizeof(struct client *));

    sessions.heartbeat = heap_init(capacity, minheap_key_cmp);

    if (!sessions.sid_to_client)
        return -1;
    if (!sessions.clients) {
        mem_free(sessions.sid_to_client);
        return -1;
    }
    if (!sessions.heartbeat) {
        mem_free(sessions.sid_to_client);
        mem_free(sessions.clients);
        return -1;
    }

    return 0;
}

int ses_add_new_client(int fd, const char *sid) {
    if (fd >= sessions.capacity)
        return -1;

    log_debug("ses_add_new_client,fd:%d,sid:%s\n", fd, sid);

    struct client *c = (struct client *) mem_malloc(sizeof(struct client));
    if (!c)
        return -1;

    c->fd = fd;
    strcpy(c->sid, sid);
    c->heartbeat = util_get_timestamp();
    util_get_fd_ip_port(fd, c->ip, &c->port);
    c->heartbeat_in_ses = heap_insert(sessions.heartbeat,
                                      (void *) c->heartbeat, (void *) c);

    sessions.clients[fd] = c;

    if (hashmap_set(sessions.sid_to_client, (void *) c->sid, (void *) c)
        == HASHMAP_OK) {
        //TODO
        sessions.size++;
        return 0;
    }

    return -1;
}

int ses_del_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return -1;

    struct client *client = sessions.clients[fd];
    sessions.clients[fd] = NULL;
    hashmap_delete(sessions.sid_to_client, (void *) client->sid, 0, 0);
    minheap_element_del(sessions.heartbeat, client->heartbeat_in_ses);

    log_debug("ses_del_client_by_fd,fd:%d,sid:%s\n", fd, client->sid);

    mem_free(client);
    sessions.size--;
}

int ses_del_client_by_sid(const char *sid) {
    if (!sid)
        return -1;

    struct client *client = NULL;
    if (hashmap_get(sessions.sid_to_client, (void *) sid, (void **) &client)
        == HASHMAP_OK) {
        sessions.size--;
        return ses_del_client_by_fd(client->fd);
    }
    return -1;
}

struct client *ses_get_client_by_sid(const char *sid) {
    if (!sid || !sessions.sid_to_client)
        return NULL;

    struct client *client = NULL;
    hashmap_get(sessions.sid_to_client, (void *) sid, (void **) &client);
    return client;
}

struct client *ses_get_client_by_fd(int fd) {
    if (fd >= sessions.capacity)
        return NULL;

    return sessions.clients[fd];
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
    struct client *c = sessions.clients[fd];
    if (!c)
        return -1;

    c->heartbeat = util_get_timestamp();
    c->heartbeat_in_ses = minheap_update(sessions.heartbeat,
                                         (void *) c->heartbeat_in_ses,
                                         (void *) c->heartbeat,
                                         minheap_key_update);
    return 0;
}

struct client **ses_get_clients(int *size) {
    *size = sessions.size;
    return sessions.clients;
}

int ses_handle_timeout_client(struct reactor_base *ae, void *data) {
    UTIL_NOTUSED(ae);
    if (heap_size(sessions.heartbeat) == 0)
        return 0;
    int current_timestamp = util_get_timestamp();
    int *k = NULL;
    struct client *c = NULL;
    do {
        heap_get_root(sessions.heartbeat, (void **) &k, (void **) &c);
        if (k == NULL || c == NULL)
            break;
        int last_ping_timestamp = (int) k;
        int heartbeat_timeout = (int) data;
        if (current_timestamp - last_ping_timestamp < heartbeat_timeout)
            break;
        log_debug("ses_handle_timeout_client,current timestamp:%d,last ping:%d,sid:%s\n",
                  current_timestamp, last_ping_timestamp, c->sid);
        ses_del_client_by_fd(c->fd);
    } while (1);
}