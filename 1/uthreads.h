#include <ucontext.h>

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt, ##args);                                                   \
        abort();                                                               \
    } while (0);

struct uthread_ctx {
    void *(*start)(void *);
    void *arg;
    void *retval;

    ucontext_t uctx;
    struct uthread_ctx *next;
};

typedef struct queue_t {
    struct uthread_ctx *first;
    struct uthread_ctx *last;
} queue_t;

struct sched_ctx {
    int init;
    char stack[64 * 1024];

    queue_t queue;
    ucontext_t uctx;
    struct uthread_ctx *current;
};

typedef struct uthread_ctx *uthread_t;

int uthread_create(uthread_t *tid, void *(*start)(void *), void *arg);
int uthread_yield();
