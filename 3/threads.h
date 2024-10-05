#ifndef PROXY_THREADS_H
#define PROXY_THREADS_H

struct clientside_ctx {
    int clientfd;
    struct cache *cache;
};

struct serverside_ctx {
    int serverfd;
    struct stream *stream;
};

void *thread_clientside(void *arg);
void *thread_serverside(void *arg);

#endif /* PROXY_THREADS_H */
