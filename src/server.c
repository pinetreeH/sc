#include "reactor.h"
#include "netevent.h"
#include "session.h"
#include "transport.h"
#include "util.h"
#include <stdlib.h>

int main(int argc, char **args) {
    struct reactor_base *base = NULL;
    base = ae_init(10);
    if (!base)
        exit(EXIT_FAILURE);

    int ping_interval = 10000;
    int ping_timeout = 20000;
    tra_conf_init(ping_interval, ping_timeout);
    char *ip = "127.0.0.1";
    int port = 5074;
    int sockfd = net_init_socket(ip, port);
    util_set_fd_nonblocking(sockfd);
    ae_add_net_event(base, sockfd, AE_EVENT_READ,
                     net_server_accpet, NULL,
                     "net_server_accpet");

    tra_http_parse_init();
    default_sio_packet_init();
    ses_init(10);
    ae_run(base);

    ae_del(base);

    return EXIT_SUCCESS;
}