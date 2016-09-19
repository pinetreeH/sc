cmake_minimum_required(VERSION 3.6)
project(sc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES server.c reactor.h reactor.c
        netevent.h netevent.c util.h util.c
        3rd/http_parser/http_parser.h 3rd/http_parser/http_parser.c
        session.h session.c hashmap.h hashmap.c
        transport.c transport.h
        3rd/c-websocket/cWebSockets.h 3rd/c-websocket/cWebSockets.c
        3rd/c-websocket/base64.h 3rd/c-websocket/base64.c
        3rd/c-websocket/md5.h 3rd/c-websocket/md5.c
        3rd/c-websocket/sha1.c 3rd/c-websocket/sha1.h
        3rd/cJson/cJSON.c 3rd/cJson/cJSON.h
        3rd/cJson/cJSON_Utils.c 3rd/cJson/cJSON_Utils.h
        handler.c handler.h)
add_executable(sc ${SOURCE_FILES} reactor.h reactor.c
        netevent.h netevent.c util.h util.c
        session.h session.c hashmap.h hashmap.c
        transport.c transport.h
        handler.c handler.h)

target_link_libraries(sc m)