#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *blocking_thread(void *_) {
    int err;

    sigset_t sigset;

    err = sigfillset(&sigset);
    if (err) {
        perror("blocking thread: sigfillset");
        return NULL;
    }

    err = pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    if (err) {
        printf("blocking thread: pthread_sigmask: %s\n", strerror(err));
        return NULL;
    }

    while (1) {
        sleep(1);
    }

    return NULL;
}

void handler(int _) {
    char *msg = "handling thread: received SIGINT\n";

    write(STDOUT_FILENO, msg, strlen(msg));
}

void *handling_thread(void *_) {
    int err;

    struct sigaction act = {
        .sa_handler = handler,
    };

    err = sigaction(SIGINT, &act, NULL);
    if (err) {
        perror("handling thread: sigaction");
        return NULL;
    }

    while (1) {
        sleep(1);
    }

    return NULL;
}

void *waiting_thread(void *_) {
    int err;

    sigset_t set;

    err = sigemptyset(&set);
    if (err) {
        perror("waiting thread: sigemptyset");
        return NULL;
    }

    err = sigaddset(&set, SIGQUIT);
    if (err) {
        perror("waiting thread: sigaddset");
        return NULL;
    }

    err = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (err) {
        printf("waiting thread: pthread_sigmask: %s\n", strerror(err));
        return NULL;
    }

    int sig;

    err = sigwait(&set, &sig);
    if (err) {
        perror("waiting thread: sigwait");
        return NULL;
    }

    printf("waiting thread: received SIG%s\n", sigabbrev_np(sig));

    return NULL;
}

int main() {
    int err;
    pthread_t tids[3];

    void *(*threads[3])(void *) = {
        blocking_thread, handling_thread, waiting_thread
    };

    for (int i = 0; i < 3; i++) {
        err = pthread_create(&tids[i], NULL, threads[i], NULL);
        if (err) {
            printf("pthread_create: %s\n", strerror(err));
            return -1;
        }
    }

    sleep(1);
    printf("sending signals to blocking thread...\n");

    int signals[6] = {SIGINT, SIGQUIT, SIGIO, SIGALRM, SIGCHLD};

    for (int i = 0; i < 5; i++) {
        err = pthread_kill(tids[0], signals[i]);
        if (err) {
            printf("pthread_kill on blocking thread: %s\n", strerror(err));
        }

        sleep(1);
    }

    sleep(1);
    printf("sending SIGINT to handling thread...\n");

    err = pthread_kill(tids[1], SIGINT);
    if (err) {
        printf("pthread_kill on handling thread: %s\n", strerror(err));
    }

    sleep(1);
    printf("sending SIGQUIT to waiting thread...\n");

    err = pthread_kill(tids[2], SIGQUIT);
    if (err) {
        printf("pthread_kill on waiting thread: %s\n", strerror(err));
    }

    for (int i = 0; i < 3; i++) {
        err = pthread_join(tids[i], NULL);
        if (err) {
            printf("pthread_join: %s\n", strerror(err));
            return -1;
        }
    }

    return 0;
}
