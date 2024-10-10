#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "wrappers.h"

struct queue_sync {
    pthread_mutex_t mutex;
};

void queue_sync_init(queue_t *queue) {
    queue->sync = malloc(sizeof(queue_sync_t));
    pthread_mutex_init(&queue->sync->mutex, NULL);
}

void queue_sync_destroy(queue_t *queue) {
    pthread_mutex_destroy(&queue->sync->mutex);
}

static void queue_lock(queue_t *queue) {
    pthread_mutex_lock(&queue->sync->mutex);
}

static void queue_unlock(queue_t *queue) {
    pthread_mutex_unlock(&queue->sync->mutex);
}

void queue_add_lock(queue_t *queue) {
    queue_lock(queue);
}

void queue_add_unlock(queue_t *queue) {
    queue_unlock(queue);
}

void queue_get_lock(queue_t *queue) {
    queue_lock(queue);
}

void queue_get_unlock(queue_t *queue) {
    queue_unlock(queue);
}
