//
// Created by pinetree on 16-9-10.
//

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>

#define HASHMAP_OK 0
#define HASHMAP_ERR 1
#define HASHMAP_ELEMENT_NOT_FOUND 2
#define HASHMAP_ELEMENT_FULL 3
#define HASHMAP_OUT_OF_MEMORY 4

typedef void hashmap_free_key(void *key);

typedef void hashmap_free_value(void *value);

typedef int hashmap_key_cmp(void *key1, void *key2);

typedef int hashmap_value_cmp(void *value1, void *value2);

typedef int hashmap_key_index(int map_capacity, void *key);

typedef struct _hashmap_element {
    void *key;
    void *value;
} hashmap_element;

typedef struct _hashmap {
    int capacity;
    int size;
    hashmap_element *elements;
    hashmap_key_cmp *key_cmp_fn;
    hashmap_value_cmp *value_cmp_fn;
    hashmap_free_key *free_key_fn;
    hashmap_free_value *free_value_fn;
    hashmap_key_index *key_index_fn;
} hashmap;

hashmap *hashmap_init(int capacity,
                      hashmap_key_cmp *key_cmp_fn,
                      hashmap_value_cmp *value_cmp_fn,
                      hashmap_free_key *free_key_fn,
                      hashmap_free_value *free_value_fn,
                      hashmap_key_index *key_index_fn);

int hashmap_free(hashmap *map, int free_key, int free_value);

int hashmap_set(hashmap *map, void *key, void *value);

int hashmap_get(hashmap *map, void *key, void *value);

int hashmap_delete(hashmap *map, void *key, int free_key, int free_value);

// for specific key value type
int hashmap_strkey_hashindex(int map_capacity, void *key);

int hashmap_intkey_hashindex(int map_capacity, void *key);

int hashmap_strkey_cmp(void *key1, void *key2);

int hashmap_intkey_cmp(void *key1, void *key2);


#endif
