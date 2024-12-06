#include <bits/time.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "uthreads.h"

#define STACK_SIZE (512 * 1024)
#define PAGE_SIZE 4096

static struct sched sched;
static struct uthread_ctx main_ctx;

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

void timespec_add_us(struct timespec *time, long duration_us) {
    time->tv_nsec += duration_us * 1000l;
    time->tv_sec += time->tv_nsec / 1000000000l;
    time->tv_nsec = time->tv_nsec % 1000000000l;
}

void enqueue_uthread(struct uthread_ctx *uthread) {
    if (!sched.queue.first) {
        sched.queue.first = uthread;
        sched.queue.last = uthread;
    } else {
        sched.queue.last->next = uthread;
        sched.queue.last = uthread;
    }
}

void handle_exit(struct uthread_ctx *uthread) {
    if (uthread == sched.queue.first) {
        sched.queue.first = uthread->next;
    }

    struct uthread_ctx *curr = sched.queue.first;

    while (curr) {
        if (curr->next == uthread) {
            curr->next = uthread->next;

            if (sched.queue.last == uthread) {
                sched.queue.last = curr;
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
    struct uthread_ctx *current = sched.current;

    if (current->exited) {
        struct uthread_ctx *next = current->next ?: sched.queue.first;
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
            next = sched.queue.first;
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

    sched.current = next;

    if (swapcontext(&sched.uctx, &next->uctx) < 0) {
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
    ctx->uctx.uc_link = &sched.uctx;

    makecontext(&ctx->uctx, (void(*))uthread_entry, 1, ctx);

    ctx->start = start;
    ctx->arg = arg;

    enqueue_uthread(ctx);

    *tid = ctx;

    uthread_yield();

    return 0;
}

void sched_init() {
    if (getcontext(&sched.uctx) < 0) {
        panic("getcontext");
    }

    sched.uctx.uc_stack.ss_sp = sched.stack;
    sched.uctx.uc_stack.ss_size = sizeof(sched.stack);

    makecontext(&sched.uctx, schedule, 0);

    if (getcontext(&main_ctx.uctx) < 0) {
        panic("getcontext");
    }

    enqueue_uthread(&main_ctx);

    sched.current = &main_ctx;
    sched.init = 1;
}

void uthread_yield() {
    if (!sched.init) {
        sched_init();
    }

    if (swapcontext(&sched.current->uctx, &sched.uctx) < 0) {
        panic("swapcontext");
    }
}

void uthread_sleep(long duration_s) {
    uthread_usleep(duration_s * 1000000l);
}

void uthread_usleep(long duration_us) {
    if (!sched.init) {
        sched_init();
    }

    clock_gettime(CLOCK_MONOTONIC, &sched.current->wait_until);
    timespec_add_us(&sched.current->wait_until, duration_us);
    uthread_yield();
}

int uthread_join(uthread_t tid, void **retval) {
    if (!sched.init) {
        sched_init();
    }

    if (tid->detached) {
        return -1;
    }

    if (!tid->exited) {
        sched.current->waiting_on = tid;
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
    if (!sched.init) {
        sched_init();
    }

    sched.current->detached = 1;
}
