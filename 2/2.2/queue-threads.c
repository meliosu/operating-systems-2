#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

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
        RED "ERROR: get value is %d but expected %d" NOCOLOR "\n",
        actual,
        expected
    );
}

float timeval_to_ms(struct timeval *time) {
    return (float)time->tv_sec * 1e3 + (float)time->tv_usec / 1e6;
}

void report_resources(void *thread_name) {
    char buf[1024];

    struct rusage rusage;
    int err;

    err = getrusage(RUSAGE_THREAD, &rusage);
    if (err) {
        sprintf(buf, "error getting resource usage\n");
    } else {
        float user_ms = timeval_to_ms(&rusage.ru_utime);
        float system_ms = timeval_to_ms(&rusage.ru_stime);
        float ratio = system_ms / (system_ms + user_ms) * 100.0;

        sprintf(
            buf,
            "%s: user=%.2fms, system=%.2fms, system/total=%.2f%%\n",
            (char *)thread_name,
            user_ms,
            system_ms,
            ratio
        );
    }

    write(STDOUT_FILENO, buf, strlen(buf));
}

void *reader(void *arg) {
    int err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (err) {
        panic("reader: setcanceltype: %s\n", strerror(err));
    }

    pthread_cleanup_push(report_resources, "reader");

    queue_t *queue = (queue_t *)arg;

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

    pthread_cleanup_pop(1);
    return NULL;
}

void *writer(void *arg) {
    int err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (err) {
        panic("writer: setcanceltype: %s\n", strerror(err));
    }

    pthread_cleanup_push(report_resources, "writer");

    queue_t *queue = (queue_t *)arg;

    int i = 0;

    while (1) {
        int ok = queue_add(queue, i);
        if (!ok) {
            continue;
        }

        i++;
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main() {
    pthread_t tid[2];
    queue_t *queue;
    int err;
    pthread_attr_t attr;

    queue = queue_init(QUEUE_SIZE);

    err = pthread_create(&tid[0], NULL, reader, queue);
    if (err) {
        panic("pthread_create reader: %s\n", strerror(err));
    }

    err = pthread_create(&tid[1], NULL, writer, queue);
    if (err) {
        panic("pthread_create writer: %s\n", strerror(err));
    }

    sleep(2);

    printf("Resource Usage\n");

    for (int i = 0; i < 2; i++) {
        err = pthread_cancel(tid[i]);
        if (err) {
            panic("pthread_cancel: %s\n", strerror(err));
        }
    }

    for (int i = 0; i < 2; i++) {
        void *ret;

        err = pthread_join(tid[i], &ret);
        if (err) {
            panic("pthread_join: %s\n", strerror(err));
        }

        if (ret != PTHREAD_CANCELED) {
            panic("thread not canceled?\n");
        }
    }

    printf("Queue Stats\n");
    queue_print_stats(queue);
    queue_destroy(queue);

    return 0;
}
