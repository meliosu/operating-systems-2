#define _GNU_SOURCE
#include <errno.h>
#include <linux/sched.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <syscall.h>
#include <unistd.h>

#include "1.6.h"

#define STACK_SIZE (8 * 1024 * 1024)

struct ctx {
    void *(*routine)(void *);
    void *arg;
};

int mythread_clone_entry(void *arg) {
    struct ctx *ctx = arg;

    ctx->routine(ctx->arg);

    return 0;
}

int mythread_create(
    mythread_t *thread, void *(*start_routine)(void *), void *arg
) {
    void *stack = mmap(
        NULL,
        STACK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_ANONYMOUS | MAP_STACK | MAP_PRIVATE,
        -1,
        0
    );

    if (stack == MAP_FAILED) {
        return errno;
    }

    struct ctx ctx = {
        .routine = start_routine,
        .arg = arg,
    };

    int tid = clone(
        mythread_clone_entry,
        stack + STACK_SIZE / sizeof(void *),
        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
            CLONE_SYSVSEM,
        &ctx
    );

    if (tid < 0) {
        munmap(stack, STACK_SIZE);
        return tid;
    }

    *thread = tid;

    return 0;
}
