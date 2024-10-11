#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void node_unlock(node_t *node) {
#if defined SYNC_MUTEX
    pthread_mutex_unlock(&node->mutex);

#elif defined SYNC_SPINLOCK
    pthread_spin_unlock(&node->spinlock);

#elif defined SYNC_RWLOCK
    pthread_rwlock_unlock(&node->rwlock);

#endif
}

#if defined SYNC_MUTEX || defined SYNC_SPINLOCK
static void node_lock(node_t *node) {
    #if defined SYNC_MUTEX
    pthread_mutex_lock(&node->mutex);

    #elif defined SYNC_SPINLOCK
    pthread_spin_lock(&node->spinlock);

    #endif
}
#endif

void node_lock_read(node_t *node) {
#if defined SYNC_MUTEX || defined SYNC_SPINLOCK
    node_lock(node);

#elif defined SYNC_RWLOCK
    pthread_rwlock_rdlock(&node->rwlock);

#endif
}

void node_lock_write(node_t *node) {
#if defined SYNC_MUTEX || defined SYNC_SPINLOCK
    node_lock(node);

#elif defined SYNC_RWLOCK
    pthread_rwlock_wrlock(&node->rwlock);

#endif
}

static void node_init_lock(node_t *node) {
#if defined SYNC_MUTEX
    pthread_mutex_init(&node->mutex, NULL);

#elif defined SYNC_SPINLOCK
    pthread_spin_init(&node->spinlock, 0);

#elif defined SYNC_RWLOCK
    pthread_rwlock_init(&node->rwlock, NULL);

#endif
}

static void node_destroy_lock(node_t *node) {
#if defined SYNC_MUTEX
    pthread_mutex_destroy(&node->mutex);

#elif defined SYNC_SPINLOCK
    pthread_spin_destroy(&node->spinlock);

#elif defined SYNC_RWLOCK
    pthread_rwlock_destroy(&node->rwlock);

#endif
}

static void node_fill_data(node_t *node) {
    for (int i = 0; i < 100; i++) {
        node->value[i] = 'a' + rand() % ('z' - 'a');
    }

    node->value[rand() % 100] = 0;
    node->next = NULL;
}

node_t *node_random() {
    node_t *node = malloc(sizeof(node_t));

    node_fill_data(node);
    node_init_lock(node);

    return node;
}

void queue_init(queue_t *queue, int size) {
    queue->head = NULL;

    for (int i = 0; i < size; i++) {
        node_t *curr = node_random();
        curr->next = queue->head;
        queue->head = curr;
    }
}

void queue_destroy(queue_t *queue) {
    node_t *curr = queue->head;

    while (curr) {
        node_t *next = curr->next;
        node_destroy_lock(curr);
        free(curr);
        curr = next;
    }
}
