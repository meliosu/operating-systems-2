#include <pthread.h>
#include <stdbool.h>

#include "stream.h"

void stream_init(stream_t *stream) {
    stream->buffer = NULL;
    stream->refcount = 1;
    stream->complete = false;
    stream->erred = false;

    pthread_mutex_init(&stream->mutex, NULL);
    pthread_cond_init(&stream->cond, NULL);
}

void stream_destroy(stream_t *stream) {
    if (atomic_fetch_sub(&stream->refcount, 1) == 1) {
        pthread_mutex_destroy(&stream->mutex);
        pthread_cond_destroy(&stream->cond);
    }
}

stream_t *stream_clone(stream_t *in) {
    atomic_fetch_add(&in->refcount, 1);
    return in;
}
