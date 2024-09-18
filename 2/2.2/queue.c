#define _GNU_SOURCE

#include <assert.h>
#include <pthread.h>
#include <threads.h>

#if defined SYNC_SEM
    #include <semaphore.h>
#endif

#include "queue.h"

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt, ##args);                                                   \
        abort();                                                               \
    } while (0);

thread_local int err;

void *qmonitor(void *arg) {
    queue_t *queue = (queue_t *)arg;

    while (1) {
        queue_print_stats(queue);
        sleep(1);
    }

    return NULL;
}

int queue_add(queue_t *queue, int val) {
    queue->add_attempts++;

#if defined SYNC_COND
    err = pthread_mutex_lock(&queue->mutex);
    if (err) {
        panic("queue_add: mutex_lock: %s\n", strerror(err));
    }

    while (queue->count == queue->max_count) {
        err = pthread_cond_wait(&queue->cond, &queue->mutex);
        if (err) {
            panic("queue_add: cond_wait: %s\n", strerror(err));
        }
    }

#elif defined SYNC_SPINLOCK
    err = pthread_spin_lock(&queue->spinlock);
    if (err) {
        panic("queue_add: spin_lock: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    err = pthread_mutex_lock(&queue->mutex);
    if (err) {
        panic("queue_add: mutex_lock: %s\n", strerror(err));
    }
#elif defined SYNC_SEM
    err = sem_wait(&queue->sem_empty);
    if (err) {
        panic("queue_add: sem_wait on sem_empty: %s\n", strerror(err));
    }

    err = sem_wait(&queue->sem_lock);
    if (err) {
        panic("queue_add: sem_wai on sem_lock")
    }

#endif

    int ret = 0;

    if (queue->count < queue->max_count) {
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

        ret = 1;
    }

#if defined SYNC_SPINLOCK
    err = pthread_spin_unlock(&queue->spinlock);
    if (err) {
        panic("queue_add: spin_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    err = pthread_mutex_unlock(&queue->mutex);
    if (err) {
        panic("queue_add: mutex_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_COND
    int signal = queue->count == 1;

    if (signal) {
        err = pthread_cond_signal(&queue->cond);
        if (err) {
            panic("queue_add: cond_signal: %s\n", strerror(err));
        }
    }

    err = pthread_mutex_unlock(&queue->mutex);
    if (err) {
        panic("queue_add: mutex_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_post(&queue->sem_lock);
    if (err) {
        panic("queue_add: sem_post on sem_lock: %s\n", strerror(err));
    }

    err = sem_post(&queue->sem_full);
    if (err) {
        panic("queue_add: sem_post on sem_full: %s\n", strerror(err));
    }

#endif

    return ret;
}

int queue_get(queue_t *queue, int *val) {
    queue->get_attempts++;

#if defined SYNC_SPINLOCK
    int err = pthread_spin_lock(&queue->spinlock);
    if (err) {
        panic("queue_get: spin_lock: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    int err = pthread_mutex_lock(&queue->mutex);
    if (err) {
        panic("queue_get: mutex_lock: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_wait(&queue->sem_full);
    if (err) {
        panic("queue_get: sem_wait on sem_full: %s\n", strerror(err));
    }

    err = sem_wait(&queue->sem_lock);
    if (err) {
        panic("queue_get: sem_wait on sem_lock")
    }

#endif

#if defined SYNC_COND
    err = pthread_mutex_lock(&queue->mutex);
    if (err) {
        panic("queue_get: mutex_lock: %s\n", strerror(err));
    }

    while (queue->count == 0) {
        err = pthread_cond_wait(&queue->cond, &queue->mutex);
        if (err) {
            panic("queue_get: cond_wait: %s\n", strerror(err));
        }
    }

#endif

    int ret = 0;

    if (queue->count > 0) {
        qnode_t *tmp = queue->first;

        *val = tmp->val;
        queue->first = queue->first->next;

        queue->count--;
        queue->get_count++;

        free(tmp);

        ret = 1;
    }

#if defined SYNC_SPINLOCK
    err = pthread_spin_unlock(&queue->spinlock);
    if (err) {
        panic("queue_get: spin_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_MUTEX
    err = pthread_mutex_unlock(&queue->mutex);
    if (err) {
        panic("queue_get: mutex_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_COND
    int signal = queue->count == queue->max_count - 1;

    if (signal) {
        err = pthread_cond_signal(&queue->cond);
        if (err) {
            panic("queue_get: cond_signal: %s\n", strerror(err));
        }
    }

    err = pthread_mutex_unlock(&queue->mutex);
    if (err) {
        panic("queue_get: mutex_unlock: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_post(&queue->sem_lock);
    if (err) {
        panic("queue_get: sem_post on sem_lock: %s\n", strerror(err));
    }

    err = sem_post(&queue->sem_empty);
    if (err) {
        panic("queue_get: sem_post on sem_empty: %s\n", strerror(err));
    }

#endif

    return ret;
}

queue_t *queue_init(int max_count) {
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
    err = pthread_mutex_init(&queue->mutex, NULL);
    if (err) {
        panic("mutex_init: %s\n", strerror(err));
    }

    err = pthread_cond_init(&queue->cond, NULL);
    if (err) {
        panic("cond_init: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_init(&queue->sem_lock, 0, 1);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }

    err = sem_init(&queue->sem_empty, 0, max_count);
    if (err) {
        panic("sem_init: %s\n", strerror(errno));
    }

    err = sem_init(&queue->sem_full, 0, 0);
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

    err = pthread_mutex_destroy(&q->mutex);
    if (err) {
        printf("mutex_destroy: %s\n", strerror(err));
    }

#elif defined SYNC_SEM
    err = sem_destroy(&q->sem_empty);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

    err = sem_destroy(&q->sem_full);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

    err = sem_destroy(&q->sem_lock);
    if (err) {
        printf("sem_destroy: %s\n", strerror(errno));
    }

#endif

    free(q);
}

void queue_print_stats(queue_t *queue) {
    printf(
        "%9s %9s %9s %9s %9s %9s %9s\n",
        "count",
        "add att.",
        "get att.",
        "diff",
        "add cnt.",
        "get cnt.",
        "diff"
    );

    printf(
        "%9d %9ld %9ld %9ld %9ld %9ld %9ld\n",
        queue->count,
        queue->add_attempts,
        queue->get_attempts,
        queue->add_attempts - queue->get_attempts,
        queue->add_count,
        queue->get_count,
        queue->add_count - queue->get_count
    );
}