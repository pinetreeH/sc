//
// Created by pinetree on 16-9-21.
//

#ifndef MINHEAP_H
#define MINHEAP_H

typedef struct _heap_element {
    int key;
    void *data;
} heap_element;

typedef struct _heap {
    int capacity;
    int size;
    heap_element *elements;

} minheap;

extern minheap *minheap_init(int capacity);

extern int minheap_insert(minheap *h, heap_element e);

extern int minheap_pop(minheap *h, heap_element *e);

extern int minheap_update_add(minheap *h, heap_element *e, int add);

#endif
