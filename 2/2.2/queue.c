#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>

#include "queue.h"

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
        printf("Cannot allocate memory for a queue\n");
        abort();
    }

    queue->first = NULL;
    queue->last = NULL;

    queue->max_count = max_count;
    queue->count = 0;

    queue->add_attempts = queue->get_attempts = 0;
    queue->add_count = queue->get_count = 0;

    err = pthread_create(&queue->qmonitor_tid, NULL, qmonitor, queue);
    if (err) {
        printf("queue_init: pthread_create: %s\n", strerror(err));
        abort();
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

    free(q);
}

int queue_add(queue_t *queue, int val) {
    queue->add_attempts++;

    assert(queue->count <= queue->max_count);

    if (queue->count == queue->max_count)
        return 0;

    qnode_t *new = malloc(sizeof(qnode_t));
    if (!new) {
        printf("Cannot allocate memory for new node\n");
        abort();
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
