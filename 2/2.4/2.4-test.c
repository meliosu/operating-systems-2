#include <pthread.h>
#include <stdio.h>

#include "2.4.h"

struct ctx {
    int counter;
    struct spinlock lock;
};

void *thread(void *arg) {
    struct ctx *ctx = arg;
    struct spinlock *lock = &ctx->lock;
    int *counter = &ctx->counter;

    for (int i = 0; i < 1000; i++) {
        spinlock_lock(lock);

        *counter += 1;

        spinlock_unlock(lock);
    }

    return NULL;
}

int main() {
    pthread_t tids[8];

    struct ctx ctx = {
        .counter = 0,
        .lock = spinlock_create(),
    };

    for (int i = 0; i < 8; i++) {
        pthread_create(&tids[i], NULL, thread, &ctx);
    }

    for (int i = 0; i < 8; i++) {
        pthread_join(tids[i], NULL);
    }

    printf("counter: %d\n", ctx.counter);

    return 0;
}
