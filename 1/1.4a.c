#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *thread(void *_) {
    while (1) {
        printf("hello\n");
    }

    return NULL;
}

int main() {
    pthread_t tid;
    int err;

    err = pthread_create(&tid, NULL, thread, NULL);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        return -1;
    }

    sleep(3);

    err = pthread_cancel(tid);
    if (err) {
        printf("pthread_cancel: %s\n", strerror(err));
        return -1;
    }

    void *status;

    err = pthread_join(tid, &status);
    if (err) {
        printf("pthread_join: %s\n", strerror(err));
        return -1;
    }

    assert(status == PTHREAD_CANCELED);

    return 0;
}
