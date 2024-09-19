#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "stack.h"
#include "uthreads.h"

#define STACK_SIZE (512 * 1024)

static struct sched_ctx sched_ctx = {
    .init = 0,
};

static struct uthread_ctx main_ctx;

void schedule() {
    while (1) {
        struct uthread_ctx *current = sched_ctx.current;
        struct uthread_ctx *next;

        if (current->next == NULL) {
            next = sched_ctx.queue.first;
        } else {
            next = current->next;
        }

        if (current == next) {
            printf("current == next\n");
            setcontext(&current->uctx);
            continue;
        }

        sched_ctx.current = next;

        if (swapcontext(&sched_ctx.uctx, &next->uctx) < 0) {
            panic("swapcontext");
        }
    }
}

void uthread_entry(struct uthread_ctx *ctx) {
    ctx->retval = ctx->start(ctx->arg);
}

int uthread_create(uthread_t *tid, void *(*start)(void *), void *arg) {
    void *stack = create_stack(STACK_SIZE);

    if (!stack) {
        return -1;
    }

    struct uthread_ctx *ctx = stack + STACK_SIZE - sizeof(struct uthread_ctx);

    if (getcontext(&ctx->uctx) < 0) {
        destroy_stack(stack, STACK_SIZE);
        return -1;
    }

    ctx->uctx.uc_stack.ss_sp = stack;
    ctx->uctx.uc_stack.ss_size = STACK_SIZE - sizeof(struct uthread_ctx);
    ctx->uctx.uc_link = &sched_ctx.uctx;

    makecontext(&ctx->uctx, (void(*))uthread_entry, 1, ctx);

    ctx->start = start;
    ctx->arg = arg;

    if (!sched_ctx.queue.first) {
        sched_ctx.queue.first = ctx;
        sched_ctx.queue.last = ctx;
    } else {
        sched_ctx.queue.last->next = ctx;
        sched_ctx.queue.last = ctx;
    }

    uthread_yield();

    return 0;
}

void sched_ctx_init() {
    if (getcontext(&sched_ctx.uctx) < 0) {
        panic("getcontext");
    }

    sched_ctx.uctx.uc_stack.ss_sp = sched_ctx.stack;
    sched_ctx.uctx.uc_stack.ss_size = sizeof(sched_ctx.stack);

    makecontext(&sched_ctx.uctx, schedule, 0);

    sched_ctx.init = 1;
}

void main_ctx_init() {
    if (getcontext(&main_ctx.uctx) < 0) {
        panic("getcontext");
    }

    sched_ctx.current = &main_ctx;

    if (!sched_ctx.queue.first) {
        sched_ctx.queue.first = &main_ctx;
        sched_ctx.queue.last = &main_ctx;
    } else {
        sched_ctx.queue.last->next = &main_ctx;
        sched_ctx.queue.last = &main_ctx;
    }
}

int uthread_yield() {
    if (!sched_ctx.init) {
        sched_ctx_init();
    }

    // yield was called inside of main ?
    if (sched_ctx.current == NULL) {
        main_ctx_init();
    }

    if (swapcontext(&sched_ctx.current->uctx, &sched_ctx.uctx) < 0) {
        panic("swapcontext");
    }

    return 0;
}
