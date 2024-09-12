#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

pthread_t last_pthread_self;

int global = 1;

void *mythread(void *arg) {
    static int local_static = 10;
    const int local_const = 100;
    int local = 1000;

    last_pthread_self = pthread_self();

    printf(
        "%-14s %-14s %-14s %-14s %-14s\n",
        "ids",
        "pid",
        "ppid",
        "tid",
        "pthread_self"
    );

    printf(
        "%-14s %-14d %-14d %-14d %-14lx\n",
        "",
        getpid(),
        getppid(),
        gettid(),
        pthread_self()
    );

    printf(
        "%-14s %-14s %-14s %-14s %-14s\n",
        "variables",
        "global",
        "local static",
        "local const",
        "local"
    );

    printf(
        "%-14s %-14p %-14p %-14p %-14p\n",
        "address",
        &global,
        &local_static,
        &local_const,
        &local
    );

    printf(
        "%-14s %-14d %-14d %-14d %-14d\n\n",
        "value",
        global,
        local_static,
        local_const,
        local
    );

    global += 1;
    local += 1;
    local_static += 1;

    return NULL;
}

int main() {
    printf(
        "main [%d %d %d]: Hello from main!\n\n", getpid(), getppid(), gettid()
    );

    for (int i = 0; i < 5; i++) {
        pthread_t tid;
        int err;

        err = pthread_create(&tid, NULL, mythread, NULL);
        if (err) {
            printf("main: pthread_create() failed: %s\n", strerror(err));
            return -1;
        }

        sleep(1);

        assert(pthread_equal(tid, last_pthread_self));

        sleep(3);

        // err = pthread_join(tid, NULL);
        // if (err) {
        //     printf("main: pthread_join() failed: %s\n", strerror(err));
        //     return -1;
        // }
    }

    return 0;
}
