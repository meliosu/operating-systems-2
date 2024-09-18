#ifndef __THREADS_H__
#define __THREADS_H__

#include <setjmp.h>

struct thread_ctx {
    void *(*start)(void *);
    void *arg;
    void *retval;
    jmp_buf jmp;
    int alive;
    int detached;
    int canceled;
};

#ifdef THREADS_PRIVATE_INCLUDE
typedef struct thread_ctx *thread_t;
#else
typedef unsigned long thread_t;
#endif

#define THREAD_CANCELED (void *)0x1

int thread_create(thread_t *tid, void *(*start)(void *), void *arg);
int thread_join(thread_t tid, void **retval);
void thread_detach();
void thread_cancel(thread_t tid);
void thread_testcancel();
[[noreturn]] void thread_exit(void *retval);

#endif /* __THREADS_H__ */
