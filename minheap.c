//
// Created by pinetree on 16-9-21.
//

#include "minheap.h"
#include "util.h"
#include <stdlib.h>

#define left_child_index(idx) (idx * 2 + 1)
#define right_child_index(idx) (idx * 2 + 2)
#define parent_index(idx)  ((idx - 1) / 2)
#define minheap_cmp(k1, k2) ( k1 <= k2)
#define reset_element(e)  do{e.key = 0; e.data = NULL;}while(0);

static heap_element *minheap_reorder_up(heap *h, int idx) {
    heap_element e = h->elements[idx];
    while (idx > 0) {
        int parent_idx = parent_index(idx);
        if (minheap_cmp(h->elements[parent_idx].key, h->elements[idx].key))
            break;
        h->elements[idx] = h->elements[parent_idx];
        idx = parent_idx;
    }
    h->elements[idx] = e;
    return h->elements + idx;
}

static heap_element *minheap_reorder_down(heap *h, int idx) {
    heap_element e = h->elements[idx];
    int child_idx = 0;
    child_idx = left_child_index(idx);
    while (child_idx < h->size) {
        if (child_idx + 1 < h->size &&
            h->elements[child_idx].key > h->elements[child_idx + 1].key) {
            child_idx++;
        }
        if (h->elements[child_idx].key >= e.key)
            break;
        h->elements[idx] = h->elements[child_idx];
        idx = child_idx;
        child_idx = left_child_index(idx);
    }
    h->elements[idx] = e;
    return h->elements + idx;
}

heap *minheap_init(int capacity) {
    heap *h = (heap *) mem_malloc(sizeof(heap));
    if (h) {
        h->elements = (heap_element *) mem_malloc(sizeof(heap_element));
        if (!h->elements) {
            mem_free(h);
            return NULL;
        }
        h->capacity = capacity;
        h->size = 0;
    }
    return h;
}

heap_element *minheap_insert(heap *h, heap_element e) {
    if (!h || h->size >= h->capacity - 1)
        return NULL;

    h->elements[h->size++] = e;
    return minheap_reorder_up(h, h->size - 1);
}

int minheap_pop(heap *h, heap_element *element) {
    if (!h)
        return -1;

    *element = h->elements[0];
    reset_element(h->elements[0]);
    h->elements[0] = h->elements[h->size > 0 ? h->size - 1 : 0];
    minheap_reorder_down(h, 0);
    h->size--;
    return 0;
}

heap_element *minheap_update(heap *h, heap_element *e, int newkey) {
    if (!h || !e)
        return -1;

    int oldkey = e->key;
    int add = newkey - oldkey;
    e->key = newkey;
    int idx = e - h->elements;
    if (add == 0)
        return e;
    else if (add > 0)
        return minheap_reorder_down(h, idx);
    else
        return minheap_reorder_up(h, idx);
}