cmake_minimum_required(VERSION 3.6)
project(sc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES server.c reactor.h reactor.c time_event.h
        time_event.c net.h net.c util.h util.c
        parse_http.h parse_http.c
        3rd/http_parser/http_parser.h 3rd/http_parser/http_parser.c
        session_manager.h session_manager.c hashmap.h hashmap.c)
add_executable(sc ${SOURCE_FILES} reactor.h reactor.c time_event.h time_event.c
        net.h net.c util.h util.c parse_http.h parse_http.c
        session_manager.h session_manager.c hashmap.h hashmap.c)