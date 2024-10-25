#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "log.h"
#include "net.h"
#include "proxy.h"
#include "sieve.h"

#define CACHE_SIZE 1024
#define BACKLOG 10
#define DEFAULT_PORT 1080

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
        port = DEFAULT_PORT;
    }

    err = register_interrupt_handler();
    if (err) {
        panic("failed to register interrupt handler: %s", strerror(errno));
    }

    signal(SIGPIPE, SIG_IGN);

    int server = net_listen(port, BACKLOG);
    if (server < 0) {
        panic("failed to create listening socket: %s", strerror(errno));
    }

    INFO("listening on port %d", port);

    cache_t cache;
    sieve_cache_init(&cache, CACHE_SIZE);

    struct sockaddr_in addr;
    socklen_t addrlen;

    pthread_attr_t attr;
    pthread_t tid;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    while (!should_quit) {
        int conn = accept(server, (struct sockaddr *)&addr, &addrlen);
        if (conn < 0) {
            if (errno == EINTR) {
                continue;
            }

            panic("error accepting connection: %s", strerror(errno));
        }

        client_ctx_t *ctx = malloc(sizeof(client_ctx_t));

        ctx->client = conn;
        ctx->cache = &cache;

        err = pthread_create(&tid, &attr, client_thread, ctx);
        if (err) {
            ERROR("error creating thread for client: %s", strerror(err));
            close(conn);
            free(ctx);
        }
    }

    INFO("exiting...");

    pthread_attr_destroy(&attr);
    close(server);
    sieve_cache_destroy(&cache);
}
