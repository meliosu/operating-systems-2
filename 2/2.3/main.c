#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

#define SWAP_PROBABILITY 10

int counter_increasing;
int counter_decreasing;
int counter_equal;
int counter_swapped;

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

    while (1) {
        traverse_compare(queue, &counter_increasing, increasing);
    }

    return NULL;
}

void *thread_decreasing(void *arg) {
    queue_t *queue = arg;

    while (1) {
        traverse_compare(queue, &counter_decreasing, decreasing);
    }

    return NULL;
}

void *thread_equal(void *arg) {
    queue_t *queue = arg;

    while (1) {
        traverse_compare(queue, &counter_equal, equal);
    }

    return NULL;
}

void swap_nodes(node_t *prev, node_t *a, node_t *b) {
    prev->next = b;
    a->next = b->next;
    b->next = a;
}

void traverse_permute(queue_t *queue, int *counter) {
    node_t *first = queue->head;

    pthread_mutex_lock(&first->mutex);
    node_t *second = first->next;

    pthread_mutex_lock(&second->mutex);
    node_t *third = second->next;

    while (third) {
        pthread_mutex_lock(&third->mutex);

        if (rand() < RAND_MAX / SWAP_PROBABILITY) {
            swap_nodes(first, second, third);
        }

        node_t *fourth = third->next;

        pthread_mutex_unlock(&first->mutex);

        first = second;
        second = third;
        third = fourth;
    }

    pthread_mutex_unlock(&first->mutex);
    pthread_mutex_unlock(&second->mutex);

    __sync_fetch_and_add(counter, 1);
}

void *thread_permutating(void *arg) {
    queue_t *queue = arg;

    while (1) {
        traverse_permute(queue, &counter_swapped);
    }

    return NULL;
}

void print_counters() {
    printf(
        "%10s %10s %10s %10s %10s\n", "incr.", "decr.", "eq.", "total", "swap"
    );

    printf(
        "%10d %10d %10d %10d %10d\n",
        counter_increasing,
        counter_decreasing,
        counter_equal,
        counter_increasing + counter_decreasing + counter_swapped,
        counter_swapped
    );
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

    while (1) {
        print_counters();
        sleep(1);
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
