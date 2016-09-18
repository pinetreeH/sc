#include "reactor.h"
#include "netevent.h"
#include "session.h"
#include "transport.h"
#include "util.h"
#include <stdlib.h>

int main(int argc, char **args) {
    struct reactor_base *base = NULL;
    base = reactor_base_init(10);
    if (!base)
        exit(EXIT_FAILURE);

    int ping_interval = 25000;
    int ping_timeout = 60000;
    transport_config_init(ping_interval, ping_timeout);
    char *ip = "127.0.0.1";
    int port = 5074;
    int sockfd = init_socket(ip, port);
    set_fd_nonblocking(sockfd);
    reactor_add_net_event(base, sockfd, REACTOR_EVENT_READ,
                          handle_server_accpet, NULL,
                          "handle_server_accpet");

    http_parse_setting_init();
    default_sio_packet_init();
    session_init(10);
    reactor_run(base);

    reactor_base_delete(base);

    return EXIT_SUCCESS;
}