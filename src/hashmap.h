//
// Created by pinetree on 16-9-10.
//

#ifndef SC_HASHMAP_HASHMAP_H
#define SC_HASHMAP_HASHMAP_H

#define HASHMAP_OK 0
#define HASHMAP_ERR 1
#define HASHMAP_ELEMENT_FOUND 2
#define HASHMAP_ELEMENT_NOT_FOUND 3
#define HASHMAP_ELEMENT_FULL 4
#define HASHMAP_OUT_OF_MEMORY 5
#define HASHMAP_DEFAULT_CAPACITY 128

typedef struct hashmap hashmap;

typedef struct _hashmap_iterator {
    hashmap *map;
    void *data;
} hashmap_iterator;

typedef void hashmap_free_key(void *key);

typedef void hashmap_free_value(void *value);

// return 0 if key1 == key2
typedef int hashmap_key_cmp(void *key1, void *key2);

typedef int hashmap_value_cmp(void *value1, void *value2);

typedef int hashmap_key_index(int map_capacity, void *key);

extern hashmap *hashmap_init(int capacity,
                             hashmap_key_cmp *key_cmp_fn,
                             hashmap_value_cmp *value_cmp_fn,
                             hashmap_free_key *free_key_fn,
                             hashmap_free_value *free_value_fn,
                             hashmap_key_index *key_index_fn);

extern int hashmap_free(hashmap *map);

extern int hashmap_set(hashmap *map, void *key, void *value);

extern int hashmap_get(hashmap *map, void *key, void **value);

extern int hashmap_delete(hashmap *map, void *key);

extern int hashmap_size(hashmap *map);

extern hashmap_iterator hashmap_get_iterator(hashmap *map);

extern hashmap_iterator hashmap_next(hashmap_iterator e,
                                     void **key, void **value);

extern int hashmap_valid_iterator(hashmap_iterator it);

#endif
