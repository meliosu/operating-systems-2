#include <stdio.h>
#include <unistd.h>

#include "uthreads.h"

void *thread(void *arg) {
    int num = (int)(long)arg;

    while (1) {
        printf("%s %d\n", __func__, num);

        if (num == 0) {
            uthread_sleep(5);
        } else {
            uthread_sleep(1);
        }
    }
}

int main() {
    int nthreads = 2;

    int err;
    uthread_t tid[nthreads];

    for (int i = 0; i < nthreads; i++) {
        err = uthread_create(&tid[i], thread, (void *)(long)i);
        if (err) {
            perror("uthread_create\n");
            return -1;
        }
    }

    while (1) {
        uthread_sleep(30);
    }

    return 0;
}
