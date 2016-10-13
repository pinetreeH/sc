//
// Created by pinetree on 16-9-10.
//

#include "hashmap.h"
#include "util.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAX_CHAIN_LENGTH 8

typedef struct hashmap_element {
    void *key;
    void *value;
    struct hashmap_element *pre;
    struct hashmap_element *next;
} hashmap_element;

struct hashmap {
    int capacity;
    int size;
    hashmap_element *head;
    hashmap_element *last;
    hashmap_element *elements;
    hashmap_key_cmp *key_cmp_fn;
    hashmap_value_cmp *value_cmp_fn;
    hashmap_free_key *free_key_fn;
    hashmap_free_value *free_value_fn;
    hashmap_key_index *key_index_fn;
};

static inline int need_rehash(hashmap *map) {
    return map->size >= map->capacity / 2;
}

static inline int element_space_used(hashmap_element *e) {
    return e->value != NULL;
}

static int get_hash_index(hashmap *map, void *key) {
    int idx = -1;
    if (map && map->key_index_fn)
        idx = map->key_index_fn(map->capacity, key);

    return idx;
}

static int rehash(hashmap *map) {
    if (!map)
        return HASHMAP_ERR;

    int new_capacity = 2 * map->capacity;
    hashmap_element *new_element_space = (hashmap_element *)
            mem_calloc(new_capacity, sizeof(hashmap_element));
    if (!new_element_space)
        return HASHMAP_OUT_OF_MEMORY;

    hashmap_element *pre_element_space = map->elements;
    int pre_size = map->size;
    map->elements = new_element_space;
    map->capacity = new_capacity;
    map->size = 0;
    map->head = map->last = NULL;
    int ret = HASHMAP_ERR;
    for (int i = 0; i < pre_size; i++) {
        if (element_space_used(&pre_element_space[i])) {
            ret = hashmap_set(map, &pre_element_space[i].key,
                              pre_element_space[i].value);
            if (ret != HASHMAP_OK)
                return ret;
        }
    }
    mem_free(pre_element_space);
    return HASHMAP_OK;
}

static int find_empty_space(hashmap *map, void *key, void *value,
                            int *final_idx) {
    int idx = get_hash_index(map, key);
    for (int i = 0; i < MAX_CHAIN_LENGTH; i++) {
        hashmap_element *e = &map->elements[idx + i];
        if (!element_space_used(e)) {
            *final_idx = idx + i;
            return HASHMAP_OK;
        }

        if (map->key_cmp_fn(&key, &e->key) == 0)
            return HASHMAP_ELEMENT_FOUND;
    }
    return HASHMAP_ELEMENT_FULL;
}

static int get_value(hashmap *map, void *key, void **value,
                     int *element_idx) {
    if (!map)
        return HASHMAP_ERR;

    int idx = -1;
    idx = get_hash_index(map, key);
    for (int i = 0; i < MAX_CHAIN_LENGTH; i++) {
        hashmap_element *e = &map->elements[idx + i];
        if (!element_space_used(e)) {
            continue;
        }
        if (map->key_cmp_fn(&key, e) == 0) {
            if (value)
                *value = e->value;
            if (element_idx)
                *element_idx = idx + 1;
            return HASHMAP_OK;
        }
    }
    return HASHMAP_ELEMENT_NOT_FOUND;
}


hashmap *hashmap_init(int capacity,
                      hashmap_key_cmp *key_cmp_fn,
                      hashmap_value_cmp *value_cmp_fn,
                      hashmap_free_key *free_key_fn,
                      hashmap_free_value *free_value_fn,
                      hashmap_key_index *key_index_fn) {
    hashmap *map = mem_malloc(sizeof(hashmap));
    if (!map)
        return NULL;

    int hashmap_capacity = (capacity >= HASHMAP_DEFAULT_CAPACITY ?
                            capacity : HASHMAP_DEFAULT_CAPACITY);
    map->elements = (hashmap_element *)
            mem_calloc(hashmap_capacity, sizeof(hashmap_element));
    if (!map->elements) {
        mem_free(map);
        return NULL;
    }
    map->capacity = hashmap_capacity;
    map->size = 0;
    map->head = map->last = NULL;
    map->key_cmp_fn = key_cmp_fn;
    map->value_cmp_fn = value_cmp_fn;
    map->free_key_fn = free_key_fn;
    map->free_value_fn = free_value_fn;
    map->key_index_fn = key_index_fn;
    return map;
}

int hashmap_set(hashmap *map, void *key, void *value) {
    if (!map)
        return HASHMAP_ERR;

    int idx = -1;
    if (need_rehash(map))
        rehash(map);

    int ret = find_empty_space(map, key, value, &idx);
    if (ret == HASHMAP_ELEMENT_FULL) {
        log_err("after rehash, find_empty_space still FULL !\n");
        return HASHMAP_ERR;
    } else if (ret == HASHMAP_ELEMENT_FOUND) {
        return HASHMAP_OK;
    } else {
        hashmap_element *e = &map->elements[idx];
        e->key = key;
        e->value = value;
        e->pre = map->last;
        e->next = NULL;
        if (map->last)
            map->last->next = e;
        map->last = e;
        if (!map->head)
            map->head = e;

        map->size++;
        return HASHMAP_OK;
    }
}

int hashmap_get(hashmap *map, void *key, void **value) {
    return get_value(map, key, value, NULL);
}

int hashmap_delete(hashmap *map, void *key,
                   int free_key, int free_value) {
    if (!map)
        return HASHMAP_ERR;

    int ret = HASHMAP_ERR;
    int idx = -1;
    ret = get_value(map, key, NULL, &idx);
    if (ret == HASHMAP_OK) {
        hashmap_element *e = &map->elements[idx];
        if (free_key && map->free_key_fn)
            map->free_key_fn(&e->key);
        if (free_value && map->free_value_fn)
            map->free_value_fn(&e->value);

        e->key = NULL;
        e->value = NULL;
        map->size--;

        if (e->pre)
            e->pre->next = e->next;
        if (e->next)
            e->next->pre = e->pre;
        if (e == map->last)
            map->last = e->pre;
        if (e == map->head)
            map->head = NULL;
    }
    return ret;
}

int hashmap_size(hashmap *map) {
    if (map)
        return map->size;
    return 0;
}

int hashmap_free(hashmap *map, int free_key, int free_value) {
    if (!map)
        return HASHMAP_ERR;

    if ((free_key || free_value) && map->size > 0) {
        for (int i = 0; i < map->capacity; i++) {
            hashmap_element *e = &map->elements[i];
            if (!element_space_used(e))
                continue;

            if (free_key)
                map->free_key_fn(&e->key);
            if (free_value)
                map->free_value_fn(&e->value);
        }
    }
    mem_free(map->elements);
    mem_free(map);
    return HASHMAP_OK;
}

// return next valid hashmap_element pos
hashmap_iterator hashmap_next(hashmap_iterator it,
                              void **key, void **value) {
    hashmap_iterator next_it = {it.map, NULL};
    if (hashmap_valid_iterator(it)) {
        hashmap_element *e = (hashmap_element *) it.data;
        if (key)
            *key = e->key;
        if (value)
            *value = e->value;
        next_it.data = e->next;
    }
    return next_it;
}

hashmap_iterator hashmap_get_iterator(hashmap *map) {
    hashmap_iterator it = {NULL, NULL};
    if (map) {
        it.map = map;
        it.data = map->head;
    }
    return it;
}

int hashmap_valid_iterator(hashmap_iterator it) {
    return it.map && it.data;
}
