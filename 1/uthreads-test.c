#include <stdio.h>
#include <unistd.h>

#include "uthreads.h"

void *thread(void *arg) {
    int num = (int)(long)arg;

    while (1) {
        printf("%s%d\n", __func__, num);
        sleep(1);
        uthread_yield();
    }
}

int main() {
    int err;
    uthread_t tid[5];

    for (int i = 0; i < 5; i++) {
        err = uthread_create(&tid[i], thread, (void *)(long)i);
        if (err) {
            perror("uthread_create\n");
            return -1;
        }
    }

    while (1) {
        printf("%s\n", __func__);
        sleep(1);
        uthread_yield();
    }

    return 0;
}
