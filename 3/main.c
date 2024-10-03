#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "log.h"
#include "sieve.h"
#include "threads.h"

#define BACKLOG 16
#define CACHE_SIZE 1024

#define panic(fmt, args...)                                                    \
    {                                                                          \
        printf(fmt "\n", ##args);                                              \
        exit(1);                                                               \
    }

int should_quit = 0;

int setup_listening_socket(int port, int backlog) {
    int err;

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        panic("socket: %s", strerror(errno));
    }

    struct sockaddr_in addr_in = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(port),
    };

    err = bind(sock, (struct sockaddr *)&addr_in, sizeof(addr_in));
    if (err) {
        panic("bind: %s", strerror(errno));
    }

    err = listen(sock, backlog);
    if (err) {
        panic("listen: %s", strerror(errno));
    }

    return sock;
}

int get_port(int argc, char **argv) {
    int port;

    if (argc < 2) {
        panic("please provide port");
    }

    port = atoi(argv[1]);

    if (port == 0) {
        panic("invalid port number");
    }

    return port;
}

void handle_interrupt() {
    should_quit = 1;
}

void setup_interrupt() {
    int err;

    struct sigaction action = {
        .sa_handler = handle_interrupt,
    };

    err = sigaction(SIGINT, &action, NULL);
    if (err) {
        panic("sigaction: %s", strerror(errno));
    }
}

int main(int argc, char **argv) {
    int err;

    init_log();

    int port = get_port(argc, argv);
    int sock = setup_listening_socket(port, BACKLOG);
    setup_interrupt();

    struct cache cache;
    sieve_cache_init(&cache, CACHE_SIZE);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (!should_quit) {
        struct sockaddr addr;
        socklen_t addrlen;

        int conn = accept(sock, &addr, &addrlen);

        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                panic("accept: %s", strerror(errno));
            }
        }

        INFO("new connection!");

        struct clientside_ctx *context = malloc(sizeof(*context));

        context->clientfd = conn;
        context->cache = &cache;

        pthread_t tid;
        err = pthread_create(&tid, &attr, thread_clientside, context);
        if (err) {
            ERROR("error creating thread: %s", strerror(err));
            close(conn);
        }
    }

    INFO("exiting...");
    pthread_attr_destroy(&attr);
    sieve_cache_destroy(&cache);
    close(sock);
    return 0;
}
