//
// Created by pinetree on 16-9-9.
//

#include "session.h"
#include "util.h"
#include "3rd/http_parser/http_parser.h"
//#include <regex.h>
#include <string.h>
#include <stdlib.h>

#define URL_MAX_LEN  256
#define STR_MAX_LEN  32

#define KEY_TRANSPORT "transport="
#define KEY_SID "sid="
#define KEY_END_FLAG '&'

static http_parser_settings settings;

static int parse_url_callback(http_parser *parser, const char *buf,
                              size_t len) {
    char url[URL_MAX_LEN] = {'\0'};
    memcpy(url, buf, len);
    log_debug("parse_url_callback,len:%d, url:%s\n", len, url);

    // try to get sid first
    char str[STR_MAX_LEN] = {'\0'};
    int ret = get_value_of_key(url, KEY_SID, KEY_END_FLAG, str);
    if (ret > 0) {
        // get s
        log_debug("sid:%s\n", str);
        get_clientinfo_by_sid(str);

    } else {
        // try to get transport
        ret = get_value_of_key(url, KEY_TRANSPORT, KEY_END_FLAG, str);
        if (ret > 0) {
            log_debug("transport:%s\n", str);
        }
    }
}


int parse_http_request(const char *buf, int bufLen) {
    http_parser *parser = (http_parser *) malloc(sizeof(http_parser));
    http_parser_init(parser, HTTP_REQUEST);
    parser->data = NULL;
    int res = http_parser_execute(parser, &settings, buf, bufLen);
    log_debug("http_parser_execute,parsed:%d, buflen:%d\n", res, bufLen);
}

void http_parse_setting_init(void) {
    settings.on_url = parse_url_callback;
}
