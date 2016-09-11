//
// Created by pinetree on 16-9-9.
//

#ifndef PARSE_HTTP_H
#define PARSE_HTTP_H

void http_parse_setting_init(void);

int parse_http_request(const char *buf, int bufLen);

#endif
