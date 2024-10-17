#include <stdatomic.h>
#include <stdlib.h>

#include "buffer.h"

buffer_t *buffer_clone(buffer_t *in) {
    atomic_fetch_add(&in->refcount, 1);
    return in;
}

buffer_t *buffer_create(int cap) {
    buffer_t *buffer = malloc(sizeof(buffer_t) + cap);
    buffer->refcount = 1;
    buffer->cap = cap;
    return buffer;
}

void buffer_destroy(buffer_t *buffer) {
    if (atomic_fetch_sub(&buffer->refcount, 1) == 1) {
        free(buffer);
    }
}
