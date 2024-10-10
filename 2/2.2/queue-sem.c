#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct queue_sync {
    sem_t sem_empty;
    sem_t sem_full;
    sem_t sem_lock;
};

void queue_sync_init(queue_t *queue) {
    int err;

    queue->sync = malloc(sizeof(queue_sync_t));

    err = sem_init(&queue->sync->sem_lock, 0, 1);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }

    err = sem_init(&queue->sync->sem_empty, 0, queue->max_count);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }

    err = sem_init(&queue->sync->sem_full, 0, 0);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }
}

void queue_sync_destroy(queue_t *queue) {
    int err;

    err = sem_destroy(&queue->sync->sem_empty);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

    err = sem_destroy(&queue->sync->sem_full);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

    err = sem_destroy(&queue->sync->sem_lock);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }
}

void queue_add_lock(queue_t *queue) {
    int err;

    err = sem_wait(&queue->sync->sem_empty);
    if (err) {
        panic("queue_add: sem_wait on sem_empty: %s\n", strerror(err));
    }

    err = sem_wait(&queue->sync->sem_lock);
    if (err) {
        panic("queue_add: sem_wait on sem_lock");
    }
}

void queue_add_unlock(queue_t *queue) {
    int err;

    err = sem_post(&queue->sync->sem_lock);
    if (err) {
        panic("queue_add: sem_post on sem_lock: %s\n", strerror(err));
    }

    err = sem_post(&queue->sync->sem_full);
    if (err) {
        panic("queue_add: sem_post on sem_full: %s\n", strerror(err));
    }
}

void queue_get_lock(queue_t *queue) {
    int err;

    err = sem_wait(&queue->sync->sem_full);
    if (err) {
        panic("queue_get: sem_wait on sem_full: %s\n", strerror(err));
    }

    err = sem_wait(&queue->sync->sem_lock);
    if (err) {
        panic("queue_get: sem_wait on sem_lock");
    }
}

void queue_get_unlock(queue_t *queue) {
    int err;

    err = sem_post(&queue->sync->sem_lock);
    if (err) {
        panic("queue_get: sem_post on sem_lock: %s\n", strerror(err));
    }

    err = sem_post(&queue->sync->sem_empty);
    if (err) {
        panic("queue_get: sem_post on sem_empty: %s\n", strerror(err));
    }
}
