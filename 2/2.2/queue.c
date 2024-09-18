#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>

#ifdef SYNC_SEM
    #include <semaphore.h>
#endif

#include "queue.h"

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt, ##args);                                                   \
        abort();                                                               \
    } while (0);

void *qmonitor(void *arg) {
    queue_t *queue = (queue_t *)arg;

    while (1) {
        queue_print_stats(queue);
        sleep(1);
    }

    return NULL;
}

queue_t *queue_init(int max_count) {
    int err;

    queue_t *queue = malloc(sizeof(queue_t));
    if (!queue) {
        panic("cannot allocate memory for a queue\n");
    }

#if defined SYNC_SPINLOCK
    err = pthread_spin_init(&queue->spinlock, 0);
    if (err) {
        panic("spin_init: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    err = pthread_mutex_init(&queue->mutex, NULL);
    if (err) {
        panic("mutex_init: %s\n", strerror(err));
    }

#elif defined SYNC_COND
    err = pthread_cond_init(&queue->cond, NULL);
    if (err) {
        panic("cond_init: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_init(&queue->semaphore, 0, 0);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }

#endif

    queue->first = NULL;
    queue->last = NULL;

    queue->max_count = max_count;
    queue->count = 0;

    queue->add_attempts = queue->get_attempts = 0;
    queue->add_count = queue->get_count = 0;

    err = pthread_create(&queue->qmonitor_tid, NULL, qmonitor, queue);
    if (err) {
        panic("queue_init: pthread_create: %s\n", strerror(err));
    }

    return queue;
}

void queue_destroy(queue_t *q) {
    qnode_t *curr = q->first;

    while (curr) {
        qnode_t *next = curr->next;
        free(curr);
        curr = next;
    }

    int err;

#if defined SYNC_SPINLOCK
    err = pthread_spin_destroy(&q->spinlock);
    if (err) {
        printf("spin_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    err = pthread_mutex_destroy(&q->mutex);
    if (err) {
        printf("mutex_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_COND
    err = pthread_cond_destroy(&q->cond);
    if (err) {
        printf("cond_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_destroy(&q->semaphore);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

#endif

    free(q);
}

int queue_add(queue_t *queue, int val) {
    queue->add_attempts++;

    assert(queue->count <= queue->max_count);

    if (queue->count == queue->max_count)
        return 0;

    qnode_t *new = malloc(sizeof(qnode_t));
    if (!new) {
        panic("cannot allocate memory for new node\n");
    }

    new->val = val;
    new->next = NULL;

    if (!queue->first)
        queue->first = queue->last = new;
    else {
        queue->last->next = new;
        queue->last = queue->last->next;
    }

    queue->count++;
    queue->add_count++;

    return 1;
}

int queue_get(queue_t *queue, int *val) {
    queue->get_attempts++;

    assert(queue->count >= 0);

    if (queue->count == 0)
        return 0;

    qnode_t *tmp = queue->first;

    *val = tmp->val;
    queue->first = queue->first->next;

    free(tmp);
    queue->count--;
    queue->get_count++;

    return 1;
}

void queue_print_stats(queue_t *queue) {
    printf(
        "queue stats: current size %d; attempts: (%ld %ld %ld); counts (%ld "
        "%ld %ld)\n",
        queue->count,
        queue->add_attempts,
        queue->get_attempts,
        queue->add_attempts - queue->get_attempts,
        queue->add_count,
        queue->get_count,
        queue->add_count - queue->get_count
    );
}
