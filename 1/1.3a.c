#include <pthread.h>
#include <stdio.h>
#include <string.h>

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
    return NULL;
}

int main() {
    pthread_t tid;
    int err;

    struct container container = {
        .string = "hello",
        .number = 42,
    };

    err = pthread_create(&tid, NULL, routine, &container);
    if (err) {
        printf("pthread_create: %s\n", strerror(err));
        return -1;
    }

    pthread_join(tid, NULL);
    if (err) {
        printf("pthread_join: %s\n", strerror(err));
        return -1;
    }

    return 0;
}
