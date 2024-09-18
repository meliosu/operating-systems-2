#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include "queue.h"

node_t *node_random() {
    node_t *node = malloc(sizeof(node_t));

    if (!node) {
        panic("OOM");
    }

    for (int i = 0; i < 100; i++) {
        node->value[i] = 'a' + rand() % ('z' - 'a');
    }

    node->value[rand() % 100] = 0;
    node->next = NULL;

    int err;

    err = pthread_mutex_init(&node->mutex, NULL);
    if (err) {
        panic("pthread_mutex_init: %s\n", strerror(err));
    }

    return node;
}

void queue_init(queue_t *queue, int size) {
    queue->head = NULL;

    node_t *curr;

    for (int i = 0; i < size; i++) {
        curr = node_random();
        curr->next = queue->head;
        queue->head = curr;
    }
}

void queue_destroy(queue_t *queue) {
    node_t *curr = queue->head;

    int err;

    while (curr) {
        node_t *next = curr->next;

        err = pthread_mutex_destroy(&curr->mutex);
        if (err) {
            printf("pthread_mutex_destroy: %s\n", strerror(err));
        }

        free(curr);

        curr = next;
    }
}
