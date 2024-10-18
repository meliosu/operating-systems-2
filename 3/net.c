#include <stdio.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "net.h"

int net_connect_remote(char *name, char *service) {
    int err;

    if (!service) {
        service = "80";
    }

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
    };

    struct addrinfo *addrs;

    err = getaddrinfo(name, service, &hints, &addrs);
    if (err) {
        return -1;
    }

    int serverfd = -1;
    struct addrinfo *curr = addrs;
    while (curr) {
        int sfd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
        if (sfd < 0) {
            curr = curr->ai_next;
            continue;
        }

        err = connect(sfd, curr->ai_addr, curr->ai_addrlen);
        if (err) {
            close(sfd);
            curr = curr->ai_next;
            continue;
        }

        serverfd = sfd;
        break;
    }

    freeaddrinfo(addrs);
    return serverfd;
}

int net_listen(int port, int backlog) {
    int err;

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        return -1;
    }

    int yes = 1;
    err = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    if (err) {
        printf("setsockopt\n");
    }

    struct sockaddr_in addr_in = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };

    err = bind(sockfd, (struct sockaddr *)&addr_in, sizeof(addr_in));
    if (err) {
        close(sockfd);
        return -1;
    }

    err = listen(sockfd, backlog);
    if (err) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}
