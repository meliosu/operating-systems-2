#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "threads.h"

void *thread_cancelable(void *_) {
    while (1) {
        sleep(1);
        thread_testcancel();
    }
}

void *thread_joinable(void *_) {
    return (void *)0xdeadbeef;
}

void *thread_detached(void *_) {
    thread_detach();
    sleep(1);
    return NULL;
}

void *thread_modifies_arg(void *arg) {
    int *res = arg;
    *res = 100;
    return NULL;
}

void inner2() {
    thread_exit("exited");
}

void inner() {
    inner2();
}

void *thread_exits(void *_) {
    inner();
    return NULL;
}

int test_arg() {
    int err;
    int res;
    thread_t tid;

    err = thread_create(&tid, thread_modifies_arg, &res);
    if (err) {
        printf("thread_create: %s\n", strerror(err));
        return -1;
    }

    err = thread_join(tid, NULL);
    if (err) {
        printf("thread_join: %s\n", strerror(err));
        return -1;
    }

    if (res != 100) {
        printf("%d expected, got %d\n", 100, res);
        return -1;
    }

    return 0;
}

int test_cancel() {
    int err;
    thread_t tid;

    err = thread_create(&tid, thread_cancelable, NULL);
    if (err) {
        printf("thread_create: %s\n", strerror(err));
        return -1;
    }

    thread_cancel(tid);

    void *retval;

    err = thread_join(tid, &retval);
    if (err) {
        printf("thread_join: %s\n", strerror(err));
        return -1;
    }

    if (retval != THREAD_CANCELED) {
        printf("THREAD_CANCELED retval was expected, got %p\n", retval);
        return -1;
    }

    return 0;
}

int test_join() {
    int err;
    thread_t tid;

    err = thread_create(&tid, thread_joinable, NULL);
    if (err) {
        printf("thread_create: %s\n", strerror(err));
        return -1;
    }

    void *retval;

    err = thread_join(tid, &retval);
    if (err) {
        printf("thread_join: %s\n", strerror(err));
        return -1;
    }

    if (retval != (void *)0xdeadbeef) {
        printf("%p retval was expected, got %p\n", (void *)0xdeadbeef, retval);
        return -1;
    }

    return 0;
}

int test_detach() {
    int err;
    thread_t tid;

    err = thread_create(&tid, thread_detached, NULL);
    if (err) {
        printf("thread_create: %s\n", strerror(err));
        return -1;
    }

    return 0;
}

int test_exit() {
    int err;
    thread_t tid;

    err = thread_create(&tid, thread_exits, NULL);
    if (err) {
        printf("thread_create: %s\n", strerror(err));
        return -1;
    }

    void *retval;

    err = thread_join(tid, &retval);
    if (err) {
        printf("thread_join: %s\n", strerror(err));
        return -1;
    }

    if (strcmp((char *)retval, "exited")) {
        printf("wrong retval, expected 'exited', got %s\n", (char *)retval);
        return -1;
    }

    return 0;
}

int main() {
    printf("Testing Join...    ");
    if (test_join()) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

    printf("Testing Detach...    ");
    if (test_detach()) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

    printf("Testing Cancel...    ");
    if (test_cancel()) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

    printf("Testing Arg Passing...    ");
    if (test_arg()) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

    printf("Testing Exit...    ");
    if (test_exit()) {
        printf("Failed\n");
    } else {
        printf("Success\n");
    }

    return 0;
}
