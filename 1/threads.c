#define _GNU_SOURCE

#include <errno.h>
#include <sched.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <syscall.h>
#include <threads.h>
#include <unistd.h>

#define THREADS_PRIVATE_INCLUDE
#include "threads.h"

#define STACK_SIZE (8 * 1024 * 1024)
#define PAGE_SIZE 4096

thread_local struct thread_ctx *self;

static void *stack_from_ctx(struct thread_ctx *ctx) {
    return (void *)ctx + sizeof(struct thread_ctx) - STACK_SIZE;
}

[[noreturn]] static void exit_thread(long status) {
    long stack_base = (long)stack_from_ctx(self);

    asm volatile("movq %0, %%rdi\n"
                 "movq %1, %%rsi\n"
                 "movq %2, %%rax\n"
                 "syscall\n" // munmap(stack_base, STACK_SIZE)
                 "movq %3, %%rdi\n"
                 "movq %4, %%rax\n"
                 "syscall\n" // exit(status)
                 :
                 : "r"(stack_base),
                   "r"((long)STACK_SIZE),
                   "r"((long)SYS_munmap),
                   "r"(status),
                   "r"((long)SYS_exit)
                 : "rdi", "rsi", "rax");

    __builtin_unreachable();
}

static int thread_entry(void *arg) {
    self = arg;

    int jumped = setjmp(self->jmp);

    if (!jumped) {
        self->retval = self->start(self->arg);
    }

    if (self->canceled) {
        self->retval = THREAD_CANCELED;
    }

    if (self->detached) {
        exit_thread(0);
    }

    return 0;
}

int thread_create(thread_t *thread_id, void *(*start)(void *), void *arg) {
    int err;

    void *stack = mmap(
        NULL,
        STACK_SIZE,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK,
        -1,
        0
    );

    if (stack == MAP_FAILED) {
        return -errno;
    }

    err = mprotect(
        stack + PAGE_SIZE, STACK_SIZE - PAGE_SIZE, PROT_READ | PROT_WRITE
    );

    if (err) {
        munmap(stack, STACK_SIZE);
        return -errno;
    }

    struct thread_ctx *ctx = stack + STACK_SIZE - sizeof(struct thread_ctx);

    ctx->start = start;
    ctx->arg = arg;

    ctx->detached = 0;
    ctx->canceled = 0;

    int tid = clone(
        thread_entry,
        stack + STACK_SIZE - sizeof(struct thread_ctx),
        CLONE_VM | CLONE_FILES | CLONE_FS | CLONE_SIGHAND | CLONE_THREAD |
            CLONE_SYSVSEM | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID,
        ctx,
        &ctx->alive,
        NULL,
        &ctx->alive
    );

    if (tid < 0) {
        munmap(stack, STACK_SIZE);
        return -errno;
    }

    *thread_id = ctx;

    return 0;
}

int thread_join(thread_t tid, void **retval) {
    int err;

    while (tid->alive) {
        continue;
    }

    if (tid->detached) {
        return -1;
    }

    if (retval) {
        *retval = tid->retval;
    }

    void *stack = stack_from_ctx(tid);

    err = munmap(stack, STACK_SIZE);
    if (err) {
        return -1;
    }

    return 0;
}

void thread_detach() {
    self->detached = 1;
}

void thread_cancel(thread_t tid) {
    tid->canceled = 1;
}

void thread_testcancel() {
    if (self->canceled) {
        longjmp(self->jmp, 1);
    }
}

[[noreturn]] void thread_exit(void *retval) {
    self->retval = retval;
    longjmp(self->jmp, 1);
}
