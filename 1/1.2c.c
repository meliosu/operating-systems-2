#include <pthread.h>
#include <stdio.h>
#include <string.h>

void *thread(void *_) {
    return "hello world";
}

int main() {
    pthread_t tid;
    int err;

    err = pthread_create(&tid, NULL, thread, NULL);
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

    printf("thread returned: %s\n", (char *)ret);

    return 0;
}
