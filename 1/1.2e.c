#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *routine(void *_) {
    int err;

    err = pthread_detach(pthread_self());
    if (err) {
        printf("pthread_detach: %s\n", strerror(err));
        return NULL;
    }

    printf("pthread ID: %lx\n", pthread_self());

    return NULL;
}

int main() {
    pthread_t tid;
    int err;

    while (1) {
        err = pthread_create(&tid, NULL, routine, NULL);
        if (err) {
            printf("pthread_create: %s\n", strerror(err));
            return -1;
        }

        sleep(1);
    }

    return 0;
}
