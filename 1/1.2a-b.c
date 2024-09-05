#include <pthread.h>
#include <stdio.h>
#include <string.h>

void *routine(void *_) {
    return (void *)42;
}

int main() {
    pthread_t tid;
    int err;

    err = pthread_create(&tid, NULL, routine, NULL);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        return -1;
    }

    void *ret;

    err = pthread_join(tid, &ret);
    if (err) {
        printf("pthread_join: %s\n", strerror(err));
        return -1;
    }

    printf("thread returned: %d\n", (int)(long)ret);

    return 0;
}
