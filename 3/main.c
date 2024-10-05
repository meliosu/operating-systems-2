#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log.h"
#include "net.h"
#include "sieve.h"
#include "threads.h"

#define CACHE_SIZE 1024
#define BACKLOG 10

#define panic(fmt, args...)                                                    \
    {                                                                          \
        printf(fmt "\n", ##args);                                              \
        exit(1);                                                               \
    }

int should_quit = 0;

static void interrupt_handler() {
    should_quit = 1;
}

static int register_interrupt_handler() {
    struct sigaction act = {
        .sa_handler = interrupt_handler,
    };

    return sigaction(SIGINT, &act, NULL);
}

static int get_port(int argc, char **argv) {
    if (argc < 2) {
        return -1;
    }

    int port = atoi(argv[1]);
    if (port == 0) {
        return -1;
    }

    return port;
}

int main(int argc, char **argv) {
    int err;
    log_init_logger();

    int port = get_port(argc, argv);
    if (port < 0) {
        panic("please provide port number");
    }

    err = register_interrupt_handler();
    if (err) {
        panic("failed to register interrupt handler: %s", strerror(errno));
    }

    int sock = net_listen(port, BACKLOG);
    if (sock < 0) {
        panic("failed to create listening socket: %s", strerror(errno));
    }

    struct cache cache;
    sieve_cache_init(&cache, CACHE_SIZE);

    char addrbuf[INET_ADDRSTRLEN];
    struct sockaddr_in addr;
    socklen_t addrlen;

    while (!should_quit) {
        int conn = accept(sock, (struct sockaddr *)&addr, &addrlen);
        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            }

            panic("error accepting connection: %s", strerror(errno));
        }

        const char *addr_str =
            inet_ntop(AF_INET, &addr.sin_addr, addrbuf, INET_ADDRSTRLEN);

        if (addr_str) {
            INFO("new connection: %s", addr_str);
        } else {
            INFO("new connection");
        }

        struct clientside_ctx *context = malloc(sizeof(struct clientside_ctx));
        context->clientfd = conn;
        context->cache = &cache;

        pthread_t tid;
        err = pthread_create(&tid, NULL, thread_clientside, context);
        if (err) {
            ERROR("pthread_create: %s", strerror(err));
        }
    }

    INFO("exiting...");

    close(sock);
    sieve_cache_destroy(&cache);
}
