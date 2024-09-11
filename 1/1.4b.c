#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *thread(void *_) {
    int err;

    err = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (err) {
        printf("pthread_setcancelype: %s\n", strerror(err));
        return NULL;
    }

    int counter = 0;

    while (1) {
        counter += 1;
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
