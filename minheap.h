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

} heap;

extern heap *minheap_init(int capacity);

extern heap_element *minheap_insert(heap *h, heap_element e);

extern int minheap_pop(heap *h, heap_element *e);

extern heap_element *minheap_update(heap *h, heap_element *e, int newkey);

#endif
