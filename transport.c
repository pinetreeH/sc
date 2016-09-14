//
// Created by pinetree on 16-9-14.
//

#include "transport.h"
#include "3rd/c-websocket/cWebSockets.h"

int ws_generate_handshake(const char *data, const int len, char *dst) {
    WEBSOCKET_generate_handshake(data, dst, len);
    return 1;
}
