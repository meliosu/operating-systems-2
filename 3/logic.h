#ifndef PROXY_LOGIC_H
#define PROXY_LOGIC_H

#include "sieve.h"
#include "stream.h"

struct client_ctx {
    struct cache *cache;
    int client;
};

struct server_ctx {
    struct stream *stream;
    int server;
};

void *client_thread(void *arg);
void *server_thread(void *arg);

int client_handler(struct cache *cache, int client);
int server_handler(struct stream *stream, int server);

#endif /* PROXY_LOGIC_H */
