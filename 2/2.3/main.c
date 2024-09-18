#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

int increasing(char *a, char *b) {
    return strlen(a) < strlen(b);
}

int decreasing(char *a, char *b) {
    return strlen(a) > strlen(b);
}

int equal(char *a, char *b) {
    return strlen(a) == strlen(b);
}

void traverse_compare(
    queue_t *queue, int *counter, int (*ordered)(char *, char *)
) {
    node_t *current = queue->head;

    pthread_mutex_lock(&current->mutex);
    node_t *next = current->next;

    while (next) {
        pthread_mutex_lock(&next->mutex);
        ordered(current->value, next->value);

        node_t *actually_next = next->next;

        pthread_mutex_unlock(&current->mutex);
        current = next;
        next = actually_next;
    }

    pthread_mutex_unlock(&current->mutex);
    *counter += 1;
}

void *thread_increasing(void *arg) {
    queue_t *queue = arg;
    int counter;

    while (1) {
        traverse_compare(queue, &counter, increasing);
    }

    return NULL;
}

void *thread_decreasing(void *arg) {
    queue_t *queue = arg;
    int counter;

    while (1) {
        traverse_compare(queue, &counter, decreasing);
    }

    return NULL;
}

void *thread_equal(void *arg) {
    queue_t *queue = arg;
    int counter;

    while (1) {
        traverse_compare(queue, &counter, equal);
    }

    return NULL;
}

void *thread_permutating(void *arg) {
    queue_t *queue = arg;

    // TODO ...

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
