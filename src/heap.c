//
// Created by pinetree on 16-9-21.
//

#include "heap.h"
#include "util.h"
#include <stdlib.h>

#define left_child_index(idx) (idx * 2 + 1)
#define right_child_index(idx) (idx * 2 + 2)
#define parent_index(idx)  ((idx - 1) / 2)
#define reset_element(e)  do{e.key = 0; e.data = NULL;}while(0);

typedef struct _heap_element {
    void *key;
    void *data;
} heap_element;

struct heap {
    int capacity;
    int size;
    heap_key_cmp *key_cmp_fn;
    heap_element *elements;
};

static heap_element *heap_reorder_up(heap *h, int idx);

static heap_element *heap_reorder_down(heap *h, int idx);

heap *heap_init(int capacity, heap_key_cmp *cmp_fn) {
    heap *h = (heap *) mem_malloc(sizeof(heap));
    if (h) {
        h->elements = (heap_element *)
                mem_calloc(capacity, sizeof(heap_element));
        if (!h->elements) {
            mem_free(h);
            return NULL;
        }
        h->capacity = capacity;
        h->size = 0;
        h->key_cmp_fn = cmp_fn;
    }
    return h;
}

void heap_del(heap *h) {
    if (h) {
        mem_free(h->elements);
        mem_free(h);
    }
}

int heap_size(heap *h) {
    if (h)
        return h->size;
    return 0;
}

void *heap_insert(heap *h, void *key, void *data) {
    if (!h || h->size >= h->capacity - 1)
        return NULL;
    heap_element e = {key, data};
    h->elements[h->size++] = e;
    return (void *) heap_reorder_up(h, h->size - 1);
}

int heap_del_root(heap *h) {
    if (!h)
        return -1;

    reset_element(h->elements[0]);
    h->elements[0] = h->elements[h->size > 0 ? h->size - 1 : 0];
    heap_reorder_down(h, 0);
    h->size--;
    return 0;
}

int heap_get_root(heap *h, void **key, void **data) {
    if (!h)
        return -1;

    *key = h->elements[0].key;
    *data = h->elements[0].data;
    return 0;
}

void *heap_root_pos(heap *h) {
    if (!h)
        return NULL;
    return (void *) h->elements;
}

static heap_element *heap_reorder_up(heap *h, int idx) {
    heap_element e = h->elements[idx];
    while (idx > 0) {
        int parent_idx = parent_index(idx);
        if (h->key_cmp_fn(h->elements[parent_idx].key, h->elements[idx].key))
            break;
        h->elements[idx] = h->elements[parent_idx];
        idx = parent_idx;
    }
    h->elements[idx] = e;
    return h->elements + idx;
}

static heap_element *heap_reorder_down(heap *h, int idx) {
    heap_element e = h->elements[idx];
    int child_idx = 0;
    child_idx = left_child_index(idx);
    while (child_idx < h->size) {
        if (child_idx + 1 < h->size &&
            !h->key_cmp_fn(h->elements[child_idx].key,
                           h->elements[child_idx + 1].key)) {
            child_idx++;
        }
        if (!h->key_cmp_fn(h->elements[child_idx].key, e.key))
            break;
        h->elements[idx] = h->elements[child_idx];
        idx = child_idx;
        child_idx = left_child_index(idx);
    }
    h->elements[idx] = e;
    return h->elements + idx;
}

int minheap_key_cmp(void *k1, void *k2) {
    return (int) k1 <= (int) k2;
}

int minheap_key_update(void *k1, void *k2) {
    return (int) k1 - (int) k2;
}

void *minheap_update(heap *h, void *elment, void *new_key,
                     heap_key_update *update_fn) {
    if (!h || !elment)
        return NULL;

    heap_element *e = (heap_element *) elment;
    void *old_key = e->key;
    int add = update_fn(new_key, old_key);
    e->key = new_key;
    int idx = e - h->elements;
    if (add == 0)
        return (void *) e;
    else if (add > 0)
        return heap_reorder_down(h, idx);
    else
        return heap_reorder_up(h, idx);
}