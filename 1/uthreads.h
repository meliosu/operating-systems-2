#include <time.h>
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

    struct timespec wait_until;
    struct uthread_ctx *waiting_on;

    ucontext_t uctx;
    struct uthread_ctx *next;

    int exited;
    int detached;
};

typedef struct queue_t {
    struct uthread_ctx *first;
    struct uthread_ctx *last;
} queue_t;

struct sched {
    int init;
    char stack[64 * 1024];

    queue_t queue;
    ucontext_t uctx;
    struct uthread_ctx *current;
};

typedef struct uthread_ctx *uthread_t;

int uthread_create(uthread_t *tid, void *(*start)(void *), void *arg);
int uthread_join(uthread_t tid, void **retval);

void uthread_yield();
void uthread_sleep(long duration_s);
void uthread_usleep(long duration_us);
void uthread_detach();
