#include <pthread.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <llhttp.h>

#include "buffer.h"
#include "http.h"
#include "log.h"
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

static void log_request(http_request_t *request) {
    INFO(
        "request: %s %s HTTP/%d.%d",
        llhttp_method_name(request->method),
        request->url,
        request->version.major,
        request->version.minor
    );
}

static void log_response(http_response_t *response) {
    INFO(
        "response: HTTP/%d.%d %d %s",
        response->version.major,
        response->version.minor,
        response->status,
        llhttp_status_name(response->status)
    )
}

static int send_from_cache(int client, stream_t *stream) {
    int sent = 0;

    while (1) {
        pthread_mutex_lock(&stream->mutex);

        while (sent == stream->len && !stream->complete && !stream->erred) {
            pthread_cond_wait(&stream->cond, &stream->mutex);
        }

        if (stream->erred) {
            pthread_mutex_unlock(&stream->mutex);
            return -1;
        }

        if (stream->complete && stream->len == sent) {
            pthread_mutex_unlock(&stream->mutex);
            break;
        }

        int len = stream->len;
        buffer_t *buffer = buffer_clone(stream->buffer);

        pthread_mutex_unlock(&stream->mutex);

        while (sent < len) {
            int n = write(client, buffer->buf + sent, len - sent);

            if (n <= 0) {
                buffer_destroy(buffer);
                return -1;
            }

            sent += n;
        }

        buffer_destroy(buffer);
    }

    return 0;
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
            free(request_buffer);
            close(client);
            return -1;
        }

        int err = http_request_parse(&request, request_buffer + rcvd, n);
        if (err) {
            ERROR("error parsing request");

            free(request_buffer);
            close(client);
            return -1;
        }

        rcvd += n;
    }

    log_request(&request);

    if (!is_allowed_request(&request)) {
        WARN("rejected request with wrong version or method");

        free(request_buffer);
        close(client);
        return -1;
    }

    stream_t *cached;
    sieve_cache_lookup(cache, request.url, &cached);

    if (cached) {
        free(request_buffer);
        err = send_from_cache(client, cached);
        close(client);
        stream_destroy(cached);
        return err;
    }

    char *host = http_host_from_url(request.url);
    if (!host) {
        free(request_buffer);
        close(client);
        return -1;
    }

    int remote = net_connect_remote(host, "80");
    if (remote < 0) {
        free(request_buffer);
        close(client);
        return -1;
    }

    free(host);

    int sent = 0;

    while (sent < rcvd) {
        int n = write(remote, request_buffer + sent, rcvd - sent);

        if (n <= 0) {
            free(request_buffer);
            close(client);
            close(remote);
            return -1;
        }

        sent += n;
    }

    free(request_buffer);

    stream_t *looked_up;
    stream_t *inserted = stream_create(RESPONSE_BUFSIZE);
    sieve_cache_lookup_or_insert(cache, request.url, &looked_up, inserted);

    if (looked_up) {
        close(remote);
        stream_destroy(inserted);
        err = send_from_cache(client, looked_up);
        close(client);
        stream_destroy(looked_up);
        return err;
    } else {
        pthread_attr_t attr;
        pthread_t tid;

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        server_ctx_t *ctx = malloc(sizeof(server_ctx_t));

        ctx->remote = remote;
        ctx->stream = stream_clone(inserted);

        err = pthread_create(&tid, &attr, server_thread, ctx);
        if (err) {
            ERROR("error creating thread for remote: %s", strerror(err));

            stream_destroy(inserted);
            stream_destroy(inserted);
            free(ctx);
            return err;
        }

        pthread_attr_destroy(&attr);

        err = send_from_cache(client, inserted);
        close(client);
        stream_destroy(inserted);
        return err;
    }
}

int server_handler(int remote, stream_t *stream) {
    int err;

    http_response_t response;
    http_response_init(&response);

    while (!response.finished) {
        if (stream->buffer->cap == stream->len) {
            pthread_mutex_lock(&stream->mutex);

            buffer_t *old = stream->buffer;
            buffer_t *new = buffer_create(old->cap * 2);
            memcpy(&new->buf, &old->buf, stream->len);
            stream->buffer = new;
            buffer_destroy(old);

            pthread_mutex_unlock(&stream->mutex);
        }

        int n = read(
            remote,
            stream->buffer->buf + stream->len,
            stream->buffer->cap - stream->len
        );

        if (n > 0) {
            err = http_response_parse(
                &response, stream->buffer->buf + stream->len, n
            );

            if (err) {
                ERROR("error parsing response");
            }
        }

        pthread_mutex_lock(&stream->mutex);

        int ret = 0;

        if (n <= 0) {
            stream->erred = true;
            ret = -1;
        } else if (err == -1) {
            stream->erred = true;
            ret = -1;
        }

        stream->len += n;

        if (response.finished) {
            stream->complete = true;
        }

        pthread_cond_broadcast(&stream->cond);
        pthread_mutex_unlock(&stream->mutex);

        if (ret) {
            close(remote);
            stream_destroy(stream);
            return ret;
        }

        if (atomic_load(&stream->refcount) == 1) {
            break;
        }
    }

    log_response(&response);

    close(remote);
    stream_destroy(stream);
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
