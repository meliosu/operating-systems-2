#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"
#include "stream.h"

stream_t *stream_create(int cap) {
    stream_t *stream = malloc(sizeof(stream_t));
    stream->buffer = buffer_create(cap);
    stream->refcount = 1;
    stream->complete = false;
    stream->erred = false;
    return stream;
}

void stream_destroy(stream_t *stream) {
    if (atomic_fetch_sub(&stream->refcount, 1) == 1) {
        pthread_mutex_destroy(&stream->mutex);
        pthread_cond_destroy(&stream->cond);
        buffer_destroy(stream->buffer);
        free(stream);
    }
}

stream_t *stream_clone(stream_t *in) {
    atomic_fetch_add(&in->refcount, 1);
    return in;
}
