#ifndef PROXY_STREAM_H
#define PROXY_STREAM_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "buffer.h"

typedef struct stream {
    buffer_t *buffer;
    int len;
    bool complete;
    bool erred;

    atomic_int refcount;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} stream_t;

stream_t *stream_clone(stream_t *in);

void stream_init(stream_t *stream);
void stream_destroy(stream_t *stream);

#endif /* PROXY_STREAM_H */
