#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "wrappers.h"

struct queue_sync {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void queue_sync_init(queue_t *queue) {
    queue->sync = malloc(sizeof(queue_sync_t));

    pthread_mutex_init(&queue->sync->mutex, NULL);
    pthread_cond_init(&queue->sync->cond, NULL);
}

void queue_sync_destroy(queue_t *queue) {
    pthread_mutex_destroy(&queue->sync->mutex);
    pthread_cond_destroy(&queue->sync->cond);
}

void queue_add_lock(queue_t *queue) {
    pthread_mutex_lock(&queue->sync->mutex);

    while (queue->count == queue->max_count) {
        pthread_cond_wait(&queue->sync->cond, &queue->sync->mutex);
        pthread_mutex_lock(&queue->sync->mutex);
    }
}

void queue_add_unlock(queue_t *queue) {
    int signal = queue->count == 1;
    pthread_mutex_unlock(&queue->sync->mutex);

    if (signal) {
        pthread_cond_signal(&queue->sync->cond);
    }
}

void queue_get_lock(queue_t *queue) {
    pthread_mutex_lock(&queue->sync->mutex);

    while (queue->count == 0) {
        pthread_cond_wait(&queue->sync->cond, &queue->sync->mutex);
        pthread_mutex_lock(&queue->sync->mutex);
    }
}

void queue_get_unlock(queue_t *queue) {
    int signal = queue->count == queue->max_count - 1;
    pthread_mutex_unlock(&queue->sync->mutex);

    if (signal) {
        pthread_cond_signal(&queue->sync->cond);
    }
}
