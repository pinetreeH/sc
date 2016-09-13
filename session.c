//
// Created by pinetree on 16-9-10.
//

#include "session.h"
#include "hashmap.h"
#include "util.h"
#include <stdlib.h>

struct session {
    int capacity;
    int sid_cnt;
    hashmap *sid_to_client; // sid is a string
};

static struct session sessions;

int session_init(int sid_max) {
    sessions.capacity = sid_max;
    sessions.sid_cnt = 0;
    sessions.sid_to_client = hashmap_init(sid_max, hashmap_strkey_cmp,
                                          NULL, NULL, NULL, hashmap_strkey_hashindex);
    if (!sessions.sid_to_client)
        return -1;

    return 0;
}

struct client_info *get_clientinfo_by_sid(char *sid) {
    if (!sid || !sessions.sid_to_client)
        return NULL;

    hashmap_key key;
    key.void_key = (void *) sid;
    struct client_info *client = NULL;
    hashmap_get(sessions.sid_to_client, &key, &client);
    return client;
}