struct thread_ctx {
    void *(*start)(void *);
    void *arg;
    void *retval;
    int alive;
    int detached;
};

typedef struct thread_ctx *thread_t;

int thread_create(thread_t *tid, void *(*start)(void *), void *arg);
int thread_join(thread_t tid, void **retval);
void thread_detach();
[[noreturn]] void thread_exit(void *retval);
