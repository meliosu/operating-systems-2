#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

void *thread_increasing(void *arg) {
    queue_t *queue = arg;

    return NULL;
}

void *thread_decreasing(void *arg) {
    queue_t *queue = arg;

    return NULL;
}

void *thread_equal(void *arg) {
    queue_t *queue = arg;

    return NULL;
}

void *thread_permutating(void *arg) {
    queue_t *queue = arg;

    return NULL;
}

int main() {
    int err;
    queue_t queue;
    pthread_t tid[6];

    queue_init(&queue, 1000);

    void *(*threads[6])(void *) = {
        thread_increasing,
        thread_decreasing,
        thread_equal,
        thread_permutating,
        thread_permutating,
        thread_permutating
    };

    for (int i = 0; i < 6; i++) {
        err = pthread_create(&tid[i], NULL, threads[i], NULL);
        if (err) {
            panic("main: pthread_create: %s\n", strerror(err));
        }
    }

    for (int i = 0; i < 6; i++) {
        err = pthread_join(tid[i], NULL);
        if (err) {
            panic("main: pthread_join: %s\n", strerror(err));
        }
    }

    queue_destroy(&queue);

    return 0;
}
