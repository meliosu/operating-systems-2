#ifndef PROXY_BUFFER_H
#define PROXY_BUFFER_H

#include <stdatomic.h>

struct buffer {
    atomic_int refcount;
    int cap;
    int len;
    char buf[];
};

struct buffer *buffer_clone(struct buffer *in);

struct buffer *buffer_create(int cap);
void buffer_destroy(struct buffer *buffer);

#endif /* PROXY_BUFFER_H */
