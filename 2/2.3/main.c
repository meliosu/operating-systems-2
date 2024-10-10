#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

#define SWAP_PROBABILITY 2

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
    queue_t *queue, int *counter, int (*order)(char *, char *)
) {
    node_t *current = queue->head;

    node_lock_read(current);
    node_t *next = current->next;

    while (next) {
        node_lock_read(next);
        order(current->value, next->value);

        node_t *actually_next = next->next;

        node_unlock(current);
        current = next;
        next = actually_next;
    }

    node_unlock(current);
    *counter += 1;
}

void *thread_increasing(void *queue) {
    while (1) {
        traverse_compare(queue, &counter_increasing, increasing);
    }
}

void *thread_decreasing(void *queue) {
    while (1) {
        traverse_compare(queue, &counter_decreasing, decreasing);
    }
}

void *thread_equal(void *queue) {
    while (1) {
        traverse_compare(queue, &counter_equal, equal);
    }
}

void swap_nodes(node_t *prev, node_t **a, node_t **b) {
    prev->next = *b;
    (*a)->next = (*b)->next;
    (*b)->next = *a;

    node_t *tmp = *a;
    *a = *b;
    *b = tmp;
}

void traverse_permute(queue_t *queue) {
    node_t *first = queue->head;

    node_lock_write(first);
    node_t *second = first->next;

    node_lock_write(second);
    node_t *third = second->next;

    while (third) {
        node_lock_write(third);

        if (rand() < RAND_MAX / SWAP_PROBABILITY) {
            swap_nodes(first, &second, &third);
        }

        node_t *fourth = third->next;

        node_unlock(first);

        first = second;
        second = third;
        third = fourth;
    }

    node_unlock(first);
    node_unlock(second);

    __sync_fetch_and_add(&counter_swapped, 1);
}

void *thread_permutating(void *queue) {
    while (1) {
        traverse_permute(queue);
    }
}

void print_counters(int elapsed) {
    if (elapsed == 0) {
        printf(
            "%2s %9s %9s %9s %9s %9s %9s\n",
            "",
            "incr.",
            "decr.",
            "eq.",
            "total",
            "swap",
            "cmp./swap"
        );
    }

    int compared = counter_increasing + counter_decreasing + counter_equal;

    float compared_to_swapped =
        (float)(compared + 1) / (float)(counter_swapped + 1);

    printf(
        "%2d %9d %9d %9d %9d %9d %9f\n",
        elapsed,
        counter_increasing,
        counter_decreasing,
        counter_equal,
        compared,
        counter_swapped,
        compared_to_swapped
    );
}

int main() {
    int err;
    queue_t queue;
    pthread_t tid[6];

    queue_init(&queue, 100);

    void *(*threads[6])(void *) = {
        thread_increasing,
        thread_decreasing,
        thread_equal,
        thread_permutating,
        thread_permutating,
        thread_permutating
    };

    for (int i = 0; i < 6; i++) {
        err = pthread_create(&tid[i], NULL, threads[i], &queue);
        if (err) {
            panic("main: pthread_create: %s\n", strerror(err));
        }
    }

    for (int elapsed = 0;; elapsed++) {
        print_counters(elapsed);
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
