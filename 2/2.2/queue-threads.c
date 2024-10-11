#define _GNU_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "queue.h"

int should_quit = 0;

void report_inconsistency(int expected, int actual) {
    printf("ERROR: expected %d, got %d\n", expected, actual);
}

float timeval_to_ms(struct timeval *time) {
    return (float)time->tv_sec * 1e3 + (float)time->tv_usec / 1e3;
}

void report_resources(void *thread_name) {
    char buf[1024];

    struct rusage rusage;
    int err;
    int len;

    err = getrusage(RUSAGE_THREAD, &rusage);
    if (err) {
        len = sprintf(buf, "error getting resource usage\n");
    } else {
        float user_ms = timeval_to_ms(&rusage.ru_utime);
        float system_ms = timeval_to_ms(&rusage.ru_stime);
        float ratio = system_ms / (system_ms + user_ms) * 100.0;

        len = sprintf(
            buf,
            "%s: user: %.0fms;  system: %.0fms;  system/total: %.2f%%\n",
            (char *)thread_name,
            user_ms,
            system_ms,
            ratio
        );
    }

    (void)(write(STDOUT_FILENO, buf, len) + 1);
}

void *reader(void *arg) {
    pthread_cleanup_push(report_resources, "reader");

    queue_t *queue = arg;
    int expected = 0;

    while (!should_quit) {
        int value = -1;

        int ok = queue_get(queue, &value);
        if (!ok) {
            continue;
        }

        if (expected != value) {
            report_inconsistency(expected, value);
        }

        expected = value + 1;
    }

    pthread_cleanup_pop(1);
    return NULL;
}

void *writer(void *arg) {
    int needs_sleep = getenv("SLEEP") != NULL;

    pthread_cleanup_push(report_resources, "writer");

    queue_t *queue = arg;
    int i = 0;

    while (!should_quit) {
        int ok = queue_add(queue, i);
        if (!ok) {
            continue;
        }

        i++;

        if (needs_sleep) {
            usleep(1);
        }
    }

    pthread_cleanup_pop(1);
    return NULL;
}

int main(int argc, char **argv) {
    int wait = 5;

    if (argc == 2) {
        wait = atoi(argv[1]) ?: 5;
    }

    int err;

    int queue_size = 1000;
    queue_t *queue = queue_create(queue_size);

    pthread_t tid[2];

    err = pthread_create(&tid[0], NULL, writer, queue);
    if (err) {
        panic("pthread_create: %s", strerror(err));
    }

    err = pthread_create(&tid[1], NULL, reader, queue);
    if (err) {
        panic("pthread_create: %s", strerror(err));
    }

    sleep(wait);

    should_quit = 1;

    for (int i = 0; i < 2; i++) {
        void *ret;

        err = pthread_join(tid[i], &ret);
        if (err) {
            panic("pthread_join: %s", strerror(err));
        }
    }

    queue_print_stats(queue);
    queue_destroy(queue);
    return 0;
}
