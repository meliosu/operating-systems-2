#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cleanup(void *memory) {
    char *string = *(char **)memory;

    if (string) {
        free(string);
    }
}

void *thread(void *memory) {
    char *string = malloc(6);

    *(char **)memory = string;

    strcpy(string, "hello");

    while (1) {
        printf("%s\n", string);
        sleep(1);
    }

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

    pthread_cleanup_push(cleanup, &string);

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

    pthread_cleanup_pop(1);

    return 0;
}
