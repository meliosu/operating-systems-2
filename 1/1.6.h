struct thread_ctx {
    void *(*start)(void *);
    void *arg;
    int exited;
    void *ret;
};

typedef struct thread_ctx *mythread_t;

int mythread_create(
    mythread_t *thread, void *(*start_routine)(void *), void *arg
);
