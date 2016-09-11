#include "reactor.h"
#include "net.h"
#include "parse_http.h"
#include "session_manager.h"
#include "util.h"
#include <stdlib.h>

int main(int argc, char **args) {
    struct reactor_base *base = NULL;
    base = reactor_base_init(10);
    if (!base) {
        exit(EXIT_FAILURE);
    }

    char *ip = "127.0.0.1";
    int port = 5074;
    int sockfd = init_socket(ip, port);
    set_fd_nonblocking(sockfd);
    reactor_add_net_event(base, sockfd, REACTOR_EVENT_READ, handle_tcp_accpet,
                          NULL, "handle_tcp_accpet");

    http_parse_setting_init();
    session_manager_init(10);
    reactor_run(base);

    return EXIT_SUCCESS;
}