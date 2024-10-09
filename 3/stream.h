#ifndef PROXY_STREAM_H
#define PROXY_STREAM_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

struct stream {
    void *data;
    int len;
    bool complete;
    bool erred;

    atomic_int refcount;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void stream_init(struct stream *stream);
void stream_destroy(struct stream *stream);

#endif /* PROXY_STREAM_H */
