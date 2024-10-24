#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "buffer.h"
#include "log.h"
#include "stream.h"

stream_t *stream_create(int cap) {
    stream_t *stream = malloc(sizeof(stream_t));
    stream->buffer = buffer_create(cap);
    stream->refcount = 1;
    stream->complete = false;
    stream->erred = false;
    stream->len = 0;
    pthread_mutex_init(&stream->mutex, NULL);
    pthread_cond_init(&stream->cond, NULL);
    return stream;
}

void stream_destroy(stream_t *stream) {
    if (atomic_fetch_sub(&stream->refcount, 1) == 1) {
        TRACE("freeing stream");

        pthread_mutex_destroy(&stream->mutex);
        pthread_cond_destroy(&stream->cond);
        buffer_destroy(stream->buffer);
        free(stream);
    }
}

stream_t *stream_clone(stream_t *in) {
    int rc = atomic_fetch_add(&in->refcount, 1);
    TRACE("stream rc: %d", rc);
    return in;
}
