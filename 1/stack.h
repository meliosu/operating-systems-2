#include <stdlib.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

void *create_stack(int size) {
    int err;

    void *stack = mmap(
        NULL, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0
    );

    if (stack == MAP_FAILED) {
        return NULL;
    }

    err = mprotect(
        (char *)stack + PAGE_SIZE, size - PAGE_SIZE, PROT_READ | PROT_WRITE
    );

    if (err) {
        munmap(stack, size);
        return NULL;
    }

    return stack;
}

void destroy_stack(void *stack, int size) {
    munmap(stack, size);
}
