#define _GNU_SOURCE
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void handler1(int _) {
    write(STDOUT_FILENO, "received SIGUSR1\n", 17);
}

void *thread1(void *_) {
    int err;

    struct sigaction act = {
        .sa_handler = handler1,
    };

    err = sigaction(SIGUSR1, &act, NULL);
    if (err) {
        perror("thread 1: sigaction");
        return NULL;
    }

    while (1) {
        sleep(1);
    }

    return NULL;
}

void handler2(int _) {
    write(STDOUT_FILENO, "received SIGUSR2\n", 17);
}

void *thread2(void *_) {
    int err;

    struct sigaction act = {
        .sa_handler = handler2,
    };

    err = sigaction(SIGUSR2, &act, NULL);
    if (err) {
        perror("thread 1: sigaction");
        return NULL;
    }

    while (1) {
        sleep(1);
    }

    return NULL;
}

int main() {
    int err;
    pthread_t tids[2];
    void *(*threads[2])(void *) = {thread1, thread2};

    for (int i = 0; i < 2; i++) {
        err = pthread_create(&tids[i], NULL, threads[i], NULL);
        if (err) {
            printf("pthread_create: %s\n", strerror(err));
            return -1;
        }
    }

    int signals[2] = {SIGUSR1, SIGUSR2};

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sleep(1);
            printf(
                "sending SIG%s to thread number %d\n",
                sigabbrev_np(signals[j]),
                i + 1
            );

            err = pthread_kill(tids[i], signals[j]);
            if (err) {
                printf("pthread_kill: %s\n", strerror(err));
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        err = pthread_join(tids[i], NULL);
        if (err) {
            printf("pthread_join: %s\n", strerror(err));
        }
    }

    return 0;
}
