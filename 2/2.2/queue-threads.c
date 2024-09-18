#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>

#include "queue.h"

#define RED "\033[41m"
#define NOCOLOR "\033[0m"
#define QUEUE_SIZE 10000

void set_cpu(int n) {
    int err;
    cpu_set_t cpuset;
    pthread_t tid = pthread_self();

    CPU_ZERO(&cpuset);
    CPU_SET(n, &cpuset);

    err = pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
    if (err) {
        printf("set_cpu: pthread_setaffinity failed for cpu %d\n", n);
        return;
    }

    printf("set_cpu: set cpu %d\n", n);
}

void report_inconsistency(int expected, int actual) {
    printf(
        RED "ERROR: get value is %d but expected - %d" NOCOLOR "\n",
        actual,
        expected
    );
}

void *reader(void *arg) {
    queue_t *queue = (queue_t *)arg;

    set_cpu(1);

    int expected = 0;

    while (1) {
        int val = -1;

        int ok = queue_get(queue, &val);
        if (!ok) {
            continue;
        }

        if (expected != val) {
            report_inconsistency(expected, val);
        }

        expected = val + 1;
    }

    return NULL;
}

void *writer(void *arg) {
    queue_t *queue = (queue_t *)arg;

    set_cpu(1);

    int i = 0;

    while (1) {
        int ok = queue_add(queue, i);
        if (!ok) {
            continue;
        }

        i++;
    }

    return NULL;
}

int main() {
    pthread_t tid[2];
    queue_t *queue;
    int err;

    queue = queue_init(QUEUE_SIZE);

    err = pthread_create(&tid[0], NULL, reader, queue);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        return -1;
    }

    err = pthread_create(&tid[1], NULL, writer, queue);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        return -1;
    }

    for (int i = 0; i < 2; i++) {
        err = pthread_join(tid[i], NULL);
        if (err) {
            printf("pthread_join: %s\n", strerror(err));
        }
    }

    return 0;
}
