#ifndef __FITOS_QUEUE_H__
#define __FITOS_QUEUE_H__

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif /* ifndef _GNU_SOURCE */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef SYNC_SEM
    #include <semaphore.h>
#endif

typedef struct _QueueNode {
    int val;
    struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
    qnode_t *first;
    qnode_t *last;

    pthread_t qmonitor_tid;

    int count;
    int max_count;

    long add_attempts;
    long get_attempts;
    long add_count;
    long get_count;

#if defined SYNC_SPINLOCK
    pthread_spinlock_t spinlock;

#elif defined SYNC_MUTEX
    pthread_mutex_t mutex;

#elif defined SYNC_COND
    pthread_cond_t cond;
    pthread_mutex_it mutex;

#elif defined SYNC_SEM
    sem_t sem;

#endif

} queue_t;

queue_t *queue_init(int max_count);
void queue_destroy(queue_t *q);

int queue_add(queue_t *q, int val);
int queue_get(queue_t *q, int *val);

void queue_print_stats(queue_t *q);

#endif /* __FITOS_QUEUE_H__ */
