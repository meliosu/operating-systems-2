#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffer.h"
#include "http.h"
#include "net.h"
#include "proxy.h"
#include "sieve.h"
#include "stream.h"

#define REQUEST_BUFSIZE (32 * 1024)
#define RESPONSE_BUFSIZE (128 * 1024)

static int is_allowed_request(http_request_t *request) {
    return request->method == HTTP_GET &&
           (request->version.major == 1 && request->version.minor == 0);
}

int client_handler(int client, cache_t *cache) {
    int err;

    http_request_t request;
    http_request_init(&request);

    int request_bufsize = REQUEST_BUFSIZE;
    void *request_buffer = malloc(request_bufsize);

    int rcvd = 0;

    while (!request.finished) {
        if (rcvd == request_bufsize) {
            request_bufsize *= 2;
            request_buffer = realloc(request_buffer, request_bufsize);
        }

        int n = read(client, request_buffer + rcvd, request_bufsize - rcvd);
        if (n <= 0) {
            // ERROR
        }

        rcvd += n;

        int err = http_request_parse(&request, request_buffer, rcvd);
        if (err == -1) {
            // ERROR
        }
    }

    if (!is_allowed_request(&request)) {
        // ERROR (kinda)
    }

    stream_t *cached;
    sieve_cache_lookup(cache, request.url, &cached);

    if (cached) {
        // send from cache
    }

    char *host = http_host_from_url(request.url);
    if (!host) {
        // ERROR
    }

    int remote = net_connect_remote(host, "80");
    if (remote < 0) {
        // ERROR
    }

    free(host);

    stream_t *looked_up;
    stream_t *inserted = stream_create(RESPONSE_BUFSIZE);
    sieve_cache_lookup_or_insert(cache, request.url, &looked_up, inserted);

    if (looked_up) {
        stream_destroy(inserted);

        // send from cache
    } else {
        pthread_attr_t attr;
        pthread_t tid;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        server_ctx_t *ctx = malloc(sizeof(server_ctx_t));

        ctx->remote = remote;
        ctx->stream = stream_clone(inserted);

        pthread_create(&tid, &attr, server_thread, ctx);
        pthread_attr_destroy(&attr);

        // send from cache
    }

    return 0;
}

int server_handler(int remote, stream_t *stream) {
    int err;

    http_response_t response;
    http_response_init(&response);

    while (!response.finished) {
        if (stream->buffer->cap == stream->len) {
            buffer_t *old = stream->buffer;
            buffer_t *new = buffer_create(old->cap * 2);
            memcpy(&new->buf, &old->buf, stream->len);
            atomic_store((atomic_long *)&stream->buffer, (long)new);
        }

        int n = read(
            remote,
            stream->buffer + stream->len,
            stream->buffer->cap - stream->len
        );

        if (n <= 0) {
            // ERROR
        }

        err = http_response_parse(
            &response, stream->buffer->buf, stream->len + n
        );

        if (err == -1) {
            // ERROR
        }

        pthread_mutex_lock(&stream->mutex);

        stream->len += n;

        pthread_cond_signal(&stream->cond);
        pthread_mutex_unlock(&stream->mutex);
    }

    return 0;
}

void *client_thread(void *arg) {
    client_ctx_t *ctx = arg;
    client_handler(ctx->client, ctx->cache);
    free(ctx);
    return NULL;
}

void *server_thread(void *arg) {
    server_ctx_t *ctx = arg;
    server_handler(ctx->remote, ctx->stream);
    free(ctx);
    return NULL;
}
