//
// Created by pinetree on 16-9-21.
//

#ifndef SC_HEAP_HEAP_H
#define SC_HEAP_HEAP_H

// minheap :return 1 if k1 <= k2; 0 if k1 > k2
typedef int heap_key_cmp(void *k1, void *k2);

typedef struct heap heap;

extern heap *heap_init(int capacity, heap_key_cmp *cmp_fn);

extern void *heap_insert(heap *h, void *key, void *data);

extern int heap_del_root(heap *h);

extern int heap_get_root(heap *h, void **key, void **data);

extern void heap_del(heap *h);

extern int heap_size(heap *h);

extern void *heap_root_pos(heap *h);

// for minheap

// return *k1 - *k2
typedef int heap_key_update(void *k1, void *k2);

extern void *minheap_update(heap *h, void *element, void *new_key,
                            heap_key_update *update_fn);

extern void *minheap_element_del(heap *h, void *element);

extern int minheap_key_cmp(void *k1, void *k2);

extern int minheap_key_update(void *new_key, void *old_key);

#endif
