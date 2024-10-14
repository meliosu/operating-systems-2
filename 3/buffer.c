#include <stdatomic.h>
#include <stdlib.h>

#include "buffer.h"

struct buffer *buffer_clone(struct buffer *in) {
    atomic_fetch_add(&in->refcount, 1);
    return in;
}

struct buffer *buffer_create(int cap) {
    struct buffer *buffer = malloc(sizeof(struct buffer) + cap);
    buffer->refcount = 1;
    buffer->cap = cap;
    buffer->len = 0;
    return buffer;
}

void buffer_destroy(struct buffer *buffer) {
    if (atomic_fetch_sub(&buffer->refcount, 1) == 1) {
        free(buffer);
    }
}
