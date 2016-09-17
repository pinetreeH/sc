//
// Created by pinetree on 16-9-9.
//

#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define LOG_MSG_MAX_LEN  1024

void log_debug(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "DEBUG";
    printf("%s> %s\n", prefix, msg);
}

void logInfo(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "INFO";
    printf("%s> %s\n", prefix, msg);
}

void log_error(const char *fmt, ...) {
    char msg[LOG_MSG_MAX_LEN];
    va_list args;
    va_start (args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end (args);
    const char *prefix = "ERROR";
    printf("%s> %s\n", prefix, msg);
}


// test: /socket.io/?EIO=3&transport=polling&t=1473479272828-0
int get_value_of_key(const char *str, const char *key, const char end_flag, char *value) {
    int idx = 0;
    char *i = NULL;
    if ((i = strstr(str, key)) != NULL) {
        char *j = i + strlen(key);
        while (*j != end_flag && *j != '\0') {
            value[idx] = *j;
            j++;
            idx++;
        }
        value[idx] = '\0';
    }
    return idx;
}

int get_timestamp(void) {
    return time(NULL);
}

int make_sid(char *sid) {
    strcpy(sid, "magic_sid");
}

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