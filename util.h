//
// Created by pinetree on 16-9-9.
//

#ifndef UTIL_H
#define UTIL_H

#define NOTUSED_PARAMETER(p) ((void) p)

void log_debug(const char *fmt, ...);

void logInfo(const char *fmt, ...);

void log_error(const char *fmt, ...);

int get_value_of_key(const char *str, const char *key, const char end_flag,
                     char *value);

int get_timestamp(void);

int make_sid(char *sid);

void *mem_malloc(int size);

void mem_free(void *data);

void *mem_calloc(int num, int size);

#endif
