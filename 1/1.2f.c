#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *routine(void *_) {
    printf("pthread ID: %lx\n", pthread_self());

    return NULL;
}

int main() {
    pthread_t tid;
    pthread_attr_t attr;
    int err;

    err = pthread_attr_init(&attr);
    if (err) {
        printf("pthread_attr_init: %s\n", strerror(err));
        return -1;
    }

    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err) {
        printf("pthread_attr_setdetachstate: %s\n", strerror(err));
        return -1;
    }

    while (1) {
        err = pthread_create(&tid, &attr, routine, NULL);
        if (err) {
            printf("pthread_create: %s\n", strerror(err));
            return -1;
        }

        sleep(1);
    }

    pthread_attr_destroy(&attr);

    return 0;
}
