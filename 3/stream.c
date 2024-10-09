#include <pthread.h>
#include <stdbool.h>

#include "stream.h"

void stream_init(struct stream *stream) {
    stream->refcount = 1;
    stream->complete = false;
    stream->erred = false;

    pthread_mutex_init(&stream->mutex, NULL);
    pthread_cond_init(&stream->cond, NULL);
}

void stream_destroy(struct stream *stream) {
    pthread_mutex_destroy(&stream->mutex);
    pthread_cond_destroy(&stream->cond);
}
