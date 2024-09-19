#include <bits/time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "uthreads.h"

#define STACK_SIZE (512 * 1024)
#define PAGE_SIZE 4096

void *create_stack(int size) {
    int err;

    void *stack = mmap(
        NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0
    );

    if (stack == MAP_FAILED) {
        return NULL;
    }

    err = mprotect(
        (char *)stack + PAGE_SIZE, size - PAGE_SIZE, PROT_READ | PROT_WRITE
    );

    if (err) {
        munmap(stack, size);
        return NULL;
    }

    return stack;
}

void destroy_stack(void *stack, int size) {
    munmap(stack, size);
}

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

long timespec_diff_us(const struct timespec *now, const struct timespec *min) {
    long duration = (min->tv_sec - now->tv_sec) * 1000000 +
                    (min->tv_nsec - now->tv_nsec) / 1000;

    return duration;
}

void handle_exit(struct uthread_ctx *uthread) {
    printf("%s\n", __func__);

    if (uthread == sched_ctx.queue.first) {
        sched_ctx.queue.first = uthread->next;
    }

    struct uthread_ctx *curr = sched_ctx.queue.first;

    while (curr) {
        if (curr->next == uthread) {
            curr->next = uthread->next;

            if (sched_ctx.queue.last == uthread) {
                sched_ctx.queue.last = curr;
            }
        }

        if (curr->waiting_on == uthread) {
            curr->waiting_on = NULL;
        }

        curr = curr->next;
    }

    if (uthread->detached) {
        destroy_stack(
            (void *)uthread + sizeof(struct uthread_ctx) - STACK_SIZE,
            STACK_SIZE
        );
    }
}

void do_schedule() {
    struct uthread_ctx *current = sched_ctx.current;

    if (current->exited) {
        struct uthread_ctx *next = current->next ?: sched_ctx.queue.first;
        handle_exit(current);
        current = next;
    }

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct timespec min = {
        .tv_sec = LONG_MAX,
        .tv_nsec = LONG_MAX,
    };

    timespec_min(&min, &current->wait_until);

    struct uthread_ctx *next = current->next;

    while (next != current) {
        // Wrap to the beginning of the queue
        if (next == NULL) {
            next = sched_ctx.queue.first;
            continue;
        }

        // Skip threads that are joining other thread
        if (next->waiting_on) {
            next = next->next;
            continue;
        }

        timespec_min(&min, &next->wait_until);

        if (timespec_less(&next->wait_until, &now)) {
            break;
        }

        next = next->next;
    }

    if (timespec_less(&now, &min)) {
        long duration = timespec_diff_us(&now, &min);
        usleep(duration);
        return;
    }

    if (next->waiting_on) {
        return;
    }

    sched_ctx.current = next;

    if (swapcontext(&sched_ctx.uctx, &next->uctx) < 0) {
        panic("swapcontext");
    }
}

void schedule() {
    while (1) {
        do_schedule();
    }
}

void uthread_entry(struct uthread_ctx *ctx) {
    ctx->retval = ctx->start(ctx->arg);
    ctx->exited = 1;
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

    ctx->exited = 0;
    ctx->waiting_on = NULL;
    ctx->detached = 0;

    ctx->wait_until.tv_sec = 0;
    ctx->wait_until.tv_nsec = 0;

    if (!sched_ctx.queue.first) {
        sched_ctx.queue.first = ctx;
        sched_ctx.queue.last = ctx;
    } else {
        sched_ctx.queue.last->next = ctx;
        sched_ctx.queue.last = ctx;
    }

    *tid = ctx;

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

int uthread_join(uthread_t tid, void **retval) {
    if (sched_ctx.current == NULL) {
        main_ctx_init();
    }

    if (tid->detached) {
        return -1;
    }

    if (!tid->exited) {
        sched_ctx.current->waiting_on = tid;
        uthread_yield();
    }

    if (retval) {
        *retval = tid->retval;
    }

    destroy_stack(
        (void *)tid + sizeof(struct uthread_ctx) - STACK_SIZE, STACK_SIZE
    );

    return 0;
}

void uthread_detach() {
    if (sched_ctx.current == NULL) {
        main_ctx_init();
    }

    sched_ctx.current->detached = 1;
}
