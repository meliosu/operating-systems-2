#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "uthreads.h"

#define STACK_SIZE (512 * 1024)
#define PAGE_SIZE 4096

static struct sched_ctx sched_ctx;

void *create_stack(int size) {
    int err;

    void *stack = mmap(
        NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0
    );

    if (stack == MAP_FAILED) {
        return NULL;
    }

    err = mprotect(stack + PAGE_SIZE, size - PAGE_SIZE, PROT_READ | PROT_WRITE);

    if (err) {
        munmap(stack, size);
        return NULL;
    }

    return stack;
}

void destroy_stack(void *stack, int size) {
    munmap(stack, size);
}

void uthread_entry(struct uthread_ctx *ctx) {
    // Add current user thread to queue
    if (!sched_ctx.queue.first) {
        sched_ctx.queue.first = ctx;
        sched_ctx.queue.last = ctx;
    } else {
        sched_ctx.queue.last->next = ctx;
        sched_ctx.queue.last = ctx;
    }
}

int uthread_create(uthread_t *tid, void *(*start)(void *), void *arg) {
    void *stack = create_stack(STACK_SIZE);

    if (!stack) {
        return -1;
    }

    struct uthread_ctx *ctx = stack + STACK_SIZE - sizeof(struct uthread_ctx);

    ctx->start = start;
    ctx->arg = arg;

    if (getcontext(&ctx->uctx) < 0) {
        destroy_stack(stack, STACK_SIZE);
        return -1;
    }

    ctx->uctx.uc_stack.ss_sp = stack;
    ctx->uctx.uc_stack.ss_size = STACK_SIZE;

    // Maybe need to pass pointer as 2 args instead
    makecontext(&ctx->uctx, (void(*))uthread_entry, 1, ctx);

    return 0;
}

void uthread_yield() {
    if (sched_ctx.current == NULL) {
        panic("current task is NULL");
    }

    /*if (swapcontext(&sched_ctx.current->uctx, &sched_ctx.ctx)) {*/
    /*    panic("swapcontext");*/
    /*}*/

    struct uthread_ctx *next;

    if (sched_ctx.current->next != NULL) {
        next = sched_ctx.current->next;
    } else {
        next = sched_ctx.queue.first;
    }

    if (next == sched_ctx.current) {
        panic("next == current");
    }

    struct uthread_ctx *prev = sched_ctx.current;

    sched_ctx.current = next;

    if (swapcontext(&prev->uctx, &next->uctx) < 0) {
        panic("swapcontext");
    }
}
