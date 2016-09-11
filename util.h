//
// Created by pinetree on 16-9-9.
//

#ifndef UTIL_H
#define UTIL_H

#define FREE(p) do{if(p)free(p); p =NULL;}while(0);

void log_debug(const char *fmt, ...);

void logInfo(const char *fmt, ...);

void log_error(const char *fmt, ...);

int get_value_of_key(const char *str, const char *key, const char endFlag, char *value);


#endif
