#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_sync {
    pthread_spinlock_t spin;
};

void queue_sync_init(queue_t *queue) {
    queue->sync = malloc(sizeof(queue_sync_t));

    int err = pthread_spin_init(&queue->sync->spin, NULL);
    if (err) {
        panic("pthread_spin_init: %s", strerror(-err));
    }
}

void queue_sync_destroy(queue_t *queue) {
    int err = pthread_spin_destroy(&queue->sync->spin);
    if (err) {
        panic("pthread_spin_init: %s", strerror(-err));
    }
}

static void queue_lock(queue_t *queue) {
    int err = pthread_spin_lock(&queue->sync->spin);
    if (err) {
        panic("pthread_spin_lock: %s", strerror(-err));
    }
}

static void queue_unlock(queue_t *queue) {
    int err = pthread_spin_unlock(&queue->sync->spin);
    if (err) {
        panic("pthread_spin_unlock: %s", strerror(-err));
    }
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
