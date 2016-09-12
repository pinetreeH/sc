//
// Created by pinetree on 16-9-12.
//

#ifndef MEMORY_H
#define MEMORY_H

void *mem_malloc(int size);

void mem_free(void *data);

void *mem_calloc(int num, int size);

#endif //SC_MEMORY_H
