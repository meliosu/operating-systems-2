#ifndef PROXY_BUFFER_H
#define PROXY_BUFFER_H

#include <stdatomic.h>

typedef struct buffer {
    atomic_int refcount;
    int cap;
    char buf[];
} buffer_t;

buffer_t *buffer_clone(buffer_t *in);

buffer_t *buffer_create(int cap);
void buffer_destroy(buffer_t *buffer);

#endif /* PROXY_BUFFER_H */
