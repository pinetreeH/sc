//
// Created by pinetree on 16-9-21.
//

#ifndef SC_MINHEAP_MINHEAP_H
#define SC_MINHEAP_MINHEAP_H

typedef int element_cmp(void *e1, void *e2);

typedef struct _heap_element {
    int key;
    void *data;
} heap_element;

typedef struct _heap {
    int capacity;
    int size;
    heap_element *elements;
    element_cmp *cmp_fn;
} heap;

extern heap *minheap_init(int capacity);

extern heap_element *minheap_insert(heap *h, heap_element e);

extern int minheap_pop(heap *h, heap_element *e);

extern heap_element *minheap_update(heap *h, heap_element *e, int newkey);

extern heap_element minheap_get_top(heap *h);

#endif
