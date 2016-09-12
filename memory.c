//
// Created by pinetree on 16-9-12.
//

#include "memory.h"
#include <stdlib.h>

void *mem_malloc(int size) {
    return malloc(size);
}

void mem_free(void *data) {
    if (data) {
        free(data);
        data = NULL;
    }
}

void *mem_calloc(int num, int size) {
    return calloc(num, size);
}