cmake_minimum_required(VERSION 3.6)
project(sc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES server.c reactor.h reactor.c timeoutevent.h
        timeoutevent.c netevent.h netevent.c util.h util.c
        parse_http.h parse_http.c
        3rd/http_parser/http_parser.h 3rd/http_parser/http_parser.c
        session.h session.c hashmap.h hashmap.c memory.h memory.c client.c client.h
        room.c room.h transport.c transport.h
        3rd/c-websocket/cWebSockets.h 3rd/c-websocket/cWebSockets.c
        3rd/c-websocket/base64.h 3rd/c-websocket/base64.c
        3rd/c-websocket/md5.h 3rd/c-websocket/md5.c
        3rd/c-websocket/sha1.c 3rd/c-websocket/sha1.h
        )
add_executable(sc ${SOURCE_FILES} reactor.h reactor.c timeoutevent.h timeoutevent.c
        netevent.h netevent.c util.h util.c parse_http.h parse_http.c
        session.h session.c hashmap.h hashmap.c memory.h memory.c client.c client.h
        room.c room.h transport.c transport.h
        )