#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

queue_t *queue_create(int max_count) {
    queue_t *queue = malloc(sizeof(queue_t));

    queue->first = NULL;
    queue->last = NULL;

    queue->max_count = max_count;
    queue->count = 0;

    queue->add_count = 0;
    queue->get_count = 0;
    queue->add_attempts = 0;
    queue->get_attempts = 0;

    queue_sync_init(queue);

    return queue;
}

void queue_destroy(queue_t *queue) {
    qnode_t *curr = queue->first;

    while (curr) {
        qnode_t *next = curr->next;
        qnode_destroy(curr);
        curr = next;
    }

    queue_sync_destroy(queue);
}

qnode_t *qnode_create(int value) {
    qnode_t *qnode = malloc(sizeof(qnode_t));
    qnode->value = value;
    qnode->next = NULL;
    return qnode;
}

void qnode_destroy(qnode_t *qnode) {
    free(qnode);
}

int queue_add(queue_t *queue, int value) {
    int ret = 0;
    qnode_t *qnode = qnode_create(value);

    queue_add_lock(queue);

    queue->add_attempts += 1;

    if (queue->count < queue->max_count) {
        if (!queue->first) {
            queue->first = queue->last = qnode;
        } else {
            queue->last = queue->last->next = qnode;
        }

        queue->count += 1;
        queue->get_count += 1;
        ret = 1;
    }

    queue_add_unlock(queue);

    if (ret == 0) {
        qnode_destroy(qnode);
    }

    return ret;
}

int queue_get(queue_t *queue, int *value) {
    int ret = 0;
    qnode_t *extracted;

    queue_get_lock(queue);

    queue->get_attempts += 1;

    if (queue->count > 0) {
        extracted = queue->first;
        *value = extracted->value;
        queue->first = extracted->next;

        queue->count -= 1;
        queue->get_count += 1;
        ret = 1;
    }

    queue_get_unlock(queue);

    if (ret == 1) {
        qnode_destroy(extracted);
    }

    return ret;
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
