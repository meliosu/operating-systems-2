#ifndef PROXY_STREAM_H
#define PROXY_STREAM_H

#include <pthread.h>

struct stream {
    void *data;
    int len;
    int complete;
    int refcount;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

void stream_init(struct stream *stream);
void stream_destroy(struct stream *stream);

#endif /* PROXY_STREAM_H */
