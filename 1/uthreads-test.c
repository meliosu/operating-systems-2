#include <stdio.h>
#include <unistd.h>

#include "uthreads.h"

#define TEST(name, test_fn)                                                    \
    do {                                                                       \
        printf("%-30s", "Testing " name "...\n");                              \
        if (test_fn()) {                                                       \
            printf("Failed\n\n");                                              \
        } else {                                                               \
            printf("Success\n\n");                                             \
        }                                                                      \
    } while (0);

void *thread_joined(void *arg) {
    *(int *)arg += 1;
    printf("thread\n");
    return NULL;
}

void *thread_sleeps(void *_) {
    for (int i = 0; i < 10; i++) {
        printf("thread\n");
        uthread_usleep(100 * 1000);
    }

    return NULL;
}

int test_join() {
    const int num_threads = 5;

    int err;
    uthread_t tid[num_threads];

    int counter = 0;

    for (int i = 0; i < num_threads; i++) {
        err = uthread_create(&tid[i], thread_joined, &counter);
        if (err) {
            return -1;
        }

        printf("created\n");
    }

    for (int i = 0; i < num_threads; i++) {
        err = uthread_join(tid[i], NULL);
        if (err) {
            return -1;
        }

        printf("joined\n");
    }

    if (counter != 5) {
        return -1;
    }

    return 0;
}

int test_sleep() {
    int err;
    uthread_t tid;

    err = uthread_create(&tid, thread_sleeps, NULL);
    if (err) {
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        printf("main\n");
        uthread_usleep(500 * 1000);
    }

    return 0;
}

int test_single_sleep() {
    for (int i = 0; i < 10; i++) {
        uthread_usleep(500 * 1000);
    }

    return 0;
}

int main() {

    TEST("Join", test_join);
    TEST("Sleep", test_sleep);
    TEST("One Thread Sleep", test_single_sleep);

    return 0;
}
