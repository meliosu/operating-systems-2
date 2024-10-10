#include <semaphore.h>
#include <stdlib.h>

#include "queue.h"
#include "wrappers.h"

struct queue_sync {
    sem_t sem_empty;
    sem_t sem_full;
    sem_t sem_lock;
};

void queue_sync_init(queue_t *queue) {
    queue->sync = malloc(sizeof(queue_sync_t));

    sem_init(&queue->sync->sem_lock, 0, 1);
    sem_init(&queue->sync->sem_empty, 0, queue->max_count);
    sem_init(&queue->sync->sem_full, 0, 0);
}

void queue_sync_destroy(queue_t *queue) {
    sem_destroy(&queue->sync->sem_empty);
    sem_destroy(&queue->sync->sem_full);
    sem_destroy(&queue->sync->sem_lock);
}

void queue_add_lock(queue_t *queue) {
    sem_wait(&queue->sync->sem_empty);
    sem_wait(&queue->sync->sem_lock);
}

void queue_add_unlock(queue_t *queue) {
    sem_post(&queue->sync->sem_lock);
    sem_post(&queue->sync->sem_full);
}

void queue_get_lock(queue_t *queue) {
    sem_wait(&queue->sync->sem_full);
    sem_wait(&queue->sync->sem_lock);
}

void queue_get_unlock(queue_t *queue) {
    sem_post(&queue->sync->sem_lock);
    sem_post(&queue->sync->sem_empty);
}
