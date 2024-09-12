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
#define PAGE_SIZE 4096

int mythread_clone_entry(void *arg) {
    struct thread_ctx *ctx = arg;

    void *ret = ctx->start(ctx->arg);

    ctx->ret = ret;
    ctx->exited = 1;

    return 0;
}

int mythread_create(
    mythread_t *thread, void *(*start_routine)(void *), void *arg
) {
    void *stack = mmap(
        NULL,
        STACK_SIZE + PAGE_SIZE,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0
    );

    if (stack == MAP_FAILED) {
        return errno;
    }

    int err = mprotect(stack + PAGE_SIZE, STACK_SIZE, PROT_READ | PROT_WRITE);

    if (err) {
        return errno;
    }

    struct thread_ctx *ctx = stack + STACK_SIZE - sizeof(struct thread_ctx);

    ctx->start = start_routine;
    ctx->arg = arg;
    ctx->exited = 0;

    int tid = clone(
        mythread_clone_entry,
        stack + PAGE_SIZE + STACK_SIZE - sizeof(struct thread_ctx),
        CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD |
            CLONE_SYSVSEM,
        ctx
    );

    if (tid < 0) {
        munmap(stack, STACK_SIZE);
        return tid;
    }

    *thread = ctx;

    return 0;
}
