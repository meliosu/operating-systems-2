#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#if defined SYNC_MUTEX
    #define SYNC_CHOSEN
#elif defined SYNC_SPINLOCK
    #if defined SYNC_CHOSEN
        #define ERROR_MULTIPLE
    #endif

    #define SYNC_CHOSEN
#elif defined SYNC_RWLOCK
    #if defined SYNC_CHOSEN
        #define ERROR_MULTIPLE
    #endif

    #define SYNC_CHOSEN
#endif

#if defined SYNC_MULTIPLE
    #error "Can't combine synchronization primitives"
#elif !defined SYNC_CHOSEN
    #error "Must define exactly one of the following: \
SYNC_MUTEX, SYNC_SPINLOCK, SYNC_RWLOCK"
#endif

void node_unlock(node_t *node) {
    int err;

#if defined SYNC_MUTEX
    err = pthread_mutex_unlock(&node->mutex);
    if (err) {
        panic("mutex_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_SPINLOCK
    err = pthread_spin_unlock(&node->spinlock);
    if (err) {
        panic("spin_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_RWLOCK
    err = pthread_rwlock_unlock(&node->rwlock);
    if (err) {
        panic("rwlock_unlock: %s\n", strerror(err));
    }

#endif
}

static void node_lock(node_t *node) {
    int err;

#if defined SYNC_MUTEX
    err = pthread_mutex_lock(&node->mutex);
    if (err) {
        panic("mutex_lock: %s\n", strerror(err));
    }

#elif defined SYNC_SPINLOCK
    err = pthread_spin_lock(&node->spinlock);
    if (err) {
        panic("spin_lock: %s\n", strerror(err));
    }

#endif
}

void node_lock_read(node_t *node) {
#if defined SYNC_MUTEX || defined SYNC_SPINLOCK
    node_lock(node);

#elif defined SYNC_RWLOCK
    int err = pthread_rwlock_rdlock(&node->rwlock);
    if (err) {
        panic("rwlock_rdlock: %s\n", strerror(err));
    }

#endif
}

void node_lock_write(node_t *node) {
#if defined SYNC_MUTEX || defined SYNC_SPINLOCK
    node_lock(node);

#elif defined SYNC_RWLOCK
    int err = pthread_rwlock_wrlock(&node->rwlock);
    if (err) {
        panic("rwlock_wrlock: %s\n", strerror(err));
    }

#endif
}

static void node_init_lock(node_t *node) {
    int err;

#if defined SYNC_MUTEX
    err = pthread_mutex_init(&node->mutex, NULL);
    if (err) {
        panic("mutex_init: %s\n", strerror(err));
    }

#elif defined SYNC_SPINLOCK
    err = pthread_spin_init(&node->spinlock, 0);
    if (err) {
        panic("spin_init: %s\n", strerror(err));
    }

#elif defined SYNC_RWLOCK
    err = pthread_rwlock_init(&node->rwlock, NULL);
    if (err) {
        panic("rwlock_init: %s\n", strerror(err));
    }

#endif
}

static void node_destroy_lock(node_t *node) {
    int err;

#if defined SYNC_MUTEX
    err = pthread_mutex_destroy(&node->mutex);
    if (err) {
        panic("mutex_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_SPINLOCK
    err = pthread_spin_destroy(&node->spinlock);
    if (err) {
        panic("spin_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_RWLOCK
    err = pthread_rwlock_destroy(&node->rwlock);
    if (err) {
        panic("rwlock_destroy: %s\n", strerror(err));
    }

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

    if (!node) {
        panic("OOM");
    }

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

    int err;

    while (curr) {
        node_t *next = curr->next;
        node_destroy_lock(curr);
        free(curr);
        curr = next;
    }
}
