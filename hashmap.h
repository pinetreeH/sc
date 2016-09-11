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


#define HASHMAP_KEYTYPE_INT 1
#define HASHMAP_KEYTYPE_STR 2

typedef union hashmap_key {
    uint64_t uint64_t_key;
    char *str_key;
} hashmap_key;

struct hashmap_element {
    hashmap_key key;
    void *value;
};

struct hashmap {
    int capacity;
    int size;
    int key_type;
    struct hashmap_element *elements;
};


struct hashmap *hashmap_init(int capacity, int key_type);

int hashmap_free(struct hashmap *map, int free_key, int free_value);

int hashmap_set(struct hashmap *map, hashmap_key key, void *value);

int hashmap_get(struct hashmap *map, hashmap_key key, void *value);

int hashmap_delete(struct hashmap *map, hashmap_key key, int free_key, int free_value);

#endif
