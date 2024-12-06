#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cleanup(void *memory) {
    if (memory) {
        free(memory);
    }
}

void *thread(void *_) {
    char *string = strdup("hello");

    pthread_cleanup_push(cleanup, string);

    while (1) {
        printf("%s\n", string);
        sleep(1);
    }

    pthread_cleanup_pop(1);

    return NULL;
}

int main() {
    pthread_t tid;
    int err;
    char *string = NULL;

    err = pthread_create(&tid, NULL, thread, &string);
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
