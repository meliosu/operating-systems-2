#ifndef PROXY_PROXY_H
#define PROXY_PROXY_H

#include "sieve.h"
#include "stream.h"

typedef struct client_ctx {
    int client;
    cache_t *cache;
} client_ctx_t;

typedef struct server_ctx {
    int remote;
    stream_t *stream;
} server_ctx_t;

int client_handler(int client, cache_t *cache);
int server_handler(int remote, stream_t *stream);

void *client_thread(void *arg);
void *server_thread(void *arg);

#endif /* PROXY_PROXY_H */
