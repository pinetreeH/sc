//
// Created by pinetree on 16-9-9.
//

#include "session.h"
#include "util.h"
#include "3rd/http_parser/http_parser.h"
#include "transport.h"
//#include <regex.h>
#include <string.h>
#include <stdlib.h>

#define URL_MAX_LEN  256
#define STR_MAX_LEN  256

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
    char sid_str[STR_MAX_LEN] = {'\0'};
    int ret = get_value_of_key(url, KEY_SID, KEY_END_FLAG, sid_str);
    if (ret > 0) {
        log_debug("sid:%s\n", sid_str);
        get_clientinfo_by_sid(sid_str);

    }
    // get transport
    char transport_str[STR_MAX_LEN] = {'\0'};
    ret = get_value_of_key(url, KEY_TRANSPORT, KEY_END_FLAG, transport_str);
    if (ret > 0) {
        log_debug("transport:%s\n", transport_str);
        if (strcmp(transport_str, TRANSPORT_WEBSOCKET) == 0) {
            char ws_resp[1024];
            ws_generate_handshake(buf, 1024, ws_resp);
            log_debug("websocket resp:%s", ws_resp);
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
