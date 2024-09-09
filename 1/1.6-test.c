#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "1.6.h"

void *entry(void *_) {
    printf("hello\n");
    return NULL;
}

int main() {
    int err;
    mythread_t thread;

    err = mythread_create(&thread, entry, NULL);
    if (err) {
        printf("error creating thread: %s\n", strerror(err));
        return -1;
    }

    sleep(1);

    return 0;
}
