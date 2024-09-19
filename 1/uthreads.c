#include <bits/time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "stack.h"
#include "uthreads.h"

#define STACK_SIZE (512 * 1024)

static struct sched_ctx sched_ctx = {
    .init = 0,
};

static struct uthread_ctx main_ctx;

int timespec_less(const struct timespec *lhs, const struct timespec *rhs) {
    if (lhs->tv_sec < rhs->tv_sec) {
        return 1;
    } else if (lhs->tv_sec == rhs->tv_sec && lhs->tv_nsec < rhs->tv_nsec) {
        return 1;
    } else {
        return 0;
    }
}

void timespec_min(struct timespec *min, const struct timespec *curr) {
    if (curr->tv_sec < min->tv_sec) {
        min->tv_sec = curr->tv_sec;
        min->tv_nsec = curr->tv_nsec;
    } else if (curr->tv_sec == min->tv_sec && curr->tv_nsec < min->tv_nsec) {
        min->tv_nsec = curr->tv_nsec;
    }
}

void usleep_from_timespecs(
    const struct timespec *min, const struct timespec *now
) {
    long duration = (min->tv_sec - now->tv_sec) * 1000000 +
                    (min->tv_nsec - now->tv_nsec) / 1000;

    usleep(duration);
}

void schedule() {
    while (1) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        struct timespec min = {
            .tv_sec = LONG_MAX,
            .tv_nsec = LONG_MAX,
        };

        struct uthread_ctx *initial = sched_ctx.current;
        struct uthread_ctx *next = initial->next;

        while (next != initial) {
            if (next == NULL) {
                next = sched_ctx.queue.first;
                continue;
            }

            if (timespec_less(&next->wait_until, &now)) {
                break;
            }

            timespec_min(&min, &next->wait_until);
            next = next->next;
        }

        if (next == initial && timespec_less(&now, &initial->wait_until)) {
            usleep_from_timespecs(&min, &now);
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

    ctx->wait_until.tv_sec = 0;
    ctx->wait_until.tv_nsec = 0;

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

void uthread_yield() {
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
}

void uthread_sleep(long duration_s) {
    uthread_usleep(duration_s * 1000000l);
}

void uthread_usleep(long duration_us) {
    if (sched_ctx.current == NULL) {
        main_ctx_init();
    }

    struct timespec *deadline = &sched_ctx.current->wait_until;

    clock_gettime(CLOCK_MONOTONIC, deadline);

    deadline->tv_nsec += duration_us * 1000l;
    deadline->tv_sec += deadline->tv_nsec / 1000000000l;
    deadline->tv_nsec = deadline->tv_nsec % 1000000000l;

    uthread_yield();
}
