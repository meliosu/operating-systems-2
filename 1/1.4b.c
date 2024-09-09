#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *thread(void *_) {
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

    if (fgetc(stdin) > 0) {
        err = pthread_cancel(tid);
        if (err) {
            printf("pthread_cancel: %s\n", strerror(err));
            return -1;
        }
    } else {
        printf("error reading from stdin\n");
        return -1;
    }

    return 0;
}
