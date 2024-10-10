#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_sync {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void queue_sync_init(queue_t *queue) {
    int err;

    queue->sync = malloc(sizeof(queue_sync_t));

    err = pthread_mutex_init(&queue->sync->mutex, NULL);
    if (err) {
        panic("pthread_mutex_init: %s", strerror(err));
    }

    err = pthread_cond_init(&queue->sync->cond, NULL);
    if (err) {
        panic("pthread_cond_init: %s", strerror(err));
    }
}

void queue_sync_destroy(queue_t *queue) {
    int err;

    err = pthread_mutex_destroy(&queue->sync->mutex);
    if (err) {
        panic("pthread_mutex_destroy: %s", strerror(err));
    }

    err = pthread_cond_destroy(&queue->sync->cond);
    if (err) {
        panic("pthread_cond_destroy: %s", strerror(err));
    }
}

void queue_add_lock(queue_t *queue) {
    int err;

    pthread_mutex_lock(&queue->sync->mutex);

    err = pthread_mutex_lock(&queue->sync->mutex);
    if (err) {
        panic("pthread_mutex_lock: %s", strerror(err));
    }

    while (queue->count == queue->max_count) {
        err = pthread_cond_wait(&queue->sync->cond, &queue->sync->mutex);
        if (err) {
            panic("pthread_cond_wait: %s", strerror(err));
        }

        err = pthread_mutex_lock(&queue->sync->mutex);
        if (err) {
            panic("pthread_mutex_lock: %s", strerror(err));
        }
    }
}

void queue_add_unlock(queue_t *queue) {
    int err;

    int signal = queue->count == 1;

    err = pthread_mutex_unlock(&queue->sync->mutex);
    if (err) {
        panic("pthread_mutex_unlock: %s", strerror(err));
    }

    if (signal) {
        err = pthread_cond_signal(&queue->sync->cond);
        if (err) {
            panic("pthread_cond_signal: %s", strerror(err));
        }
    }
}

void queue_get_lock(queue_t *queue) {
    int err;

    err = pthread_mutex_lock(&queue->sync->mutex);
    if (err) {
        panic("pthread_mutex_lock: %s", strerror(err));
    }

    while (queue->count == 0) {
        err = pthread_cond_wait(&queue->sync->cond, &queue->sync->mutex);
        if (err) {
            panic("pthread_cond_wait: %s", strerror(err));
        }

        err = pthread_mutex_lock(&queue->sync->mutex);
        if (err) {
            panic("pthread_mutex_lock: %s", strerror(err));
        }
    }
}

void queue_get_unlock(queue_t *queue) {
    int err;

    int signal = queue->count == queue->max_count - 1;

    err = pthread_mutex_unlock(&queue->sync->mutex);
    if (err) {
        panic("pthread_mutex_unlock: %s", strerror(err));
    }

    if (signal) {
        err = pthread_cond_signal(&queue->sync->cond);
        if (err) {
            panic("pthread_cond_signal: %s", strerror(err));
        }
    }
}
