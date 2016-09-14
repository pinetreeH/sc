//
// Created by pinetree on 16-9-14.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H


#define TRANSPORT_WEBSOCKET "websocket"

int ws_generate_handshake(const char *data, const int len, char *dst);

#endif //PROTOCOL_H
