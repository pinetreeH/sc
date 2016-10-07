cmake_minimum_required(VERSION 3.6)
project(sc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES
        server.c reactor.c netevent.c util.c
        session.c hashmap.c transport.c
        handler.c heap.c
        3rd/http_parser/http_parser.c
        3rd/c-websocket/cWebSockets.c
        3rd/c-websocket/base64.c
        3rd/c-websocket/md5.c
        3rd/c-websocket/sha1.c
        3rd/cJson/cJSON.c
        3rd/cJson/cJSON_Utils.c
        helper.c helper.h)
add_executable(sc ${SOURCE_FILES})

target_link_libraries(sc m)