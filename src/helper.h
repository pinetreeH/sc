//
// Created by pinetree on 10/3/16.
//

#ifndef SC_HELPER_HEL_H
#define SC_HELPER_HEL_H

// for specific key value type
extern int str_hashindex(int map_capacity, void *key);

extern int str_cmp(void *key1, void *key2);

extern int str_free(void *key);

extern int pointer_cmp(void *key1, void *key2);

extern int pointer_hashindex(int map_capacity, void *key);

#endif
