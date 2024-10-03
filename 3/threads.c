#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "threads.h"

int connect_remote(char *domain, char *port) {
    if (!port) {
        port = "80";
    }

    int err;
    struct addrinfo *addrs;

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
    };

    err = getaddrinfo(domain, port, &hints, &addrs);
    if (err) {
        return -1;
    }

    int remote_socket = -1;

    struct addrinfo *curr;
    int sock;
    for (curr = addrs; curr != NULL; curr = curr->ai_next) {
        sock = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);

        if (sock < 0) {
            continue;
        }

        err = connect(sock, curr->ai_addr, curr->ai_addrlen);
        if (err) {
            close(sock);
            continue;
        }

        remote_socket = sock;
        break;
    }

    freeaddrinfo(addrs);
    return remote_socket;
}

void *thread_clientside(void *arg) {
    return NULL;
}

void *thread_serverside(void *arg) {
    return NULL;
}
