#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct container {
    char *string;
    int number;
};

void debug_container(struct container *container) {
    printf("struct container {\n");
    printf("    string: %s,\n", container->string);
    printf("    number: %d,\n", container->number);
    printf("}\n");
}

void *routine(void *arg) {
    struct container *container = arg;
    debug_container(container);

    free(container);
    return NULL;
}

int main() {
    pthread_t tid;
    int err;
    pthread_attr_t attr;

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

    struct container *container = malloc(sizeof(struct container));

    container->string = "hello";
    container->number = 42;

    err = pthread_create(&tid, NULL, routine, container);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        free(container);
        return -1;
    }

    pthread_attr_destroy(&attr);

    sleep(1);

    return 0;
}
