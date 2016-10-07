//
// Created by pinetree on 10/3/16.
//

#ifndef SC_HELPER_HEL_H
#define SC_HELPER_HEL_H

// for specific key value type
extern int hashmap_strkey_hashindex(int map_capacity, void *key);

extern int hashmap_strkey_cmp(void *key1, void *key2);

extern int hashmap_strkey_free(void *key);

extern int hashmap_pointerkey_cmp(void *key1, void *key2);

extern int hashmap_pointerkey_hashindex(int map_capacity, void *key);

#endif
