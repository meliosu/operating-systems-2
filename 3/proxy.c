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

#define ERROR -1
#define SUCCESS 0

static int is_allowed_request(http_request_t *request) {
    return request->method == HTTP_GET &&
           (request->version.major == 1 && request->version.minor == 0);
}

static void log_request(http_request_t *request) {
    log_info(
        "request: %s %s HTTP/%d.%d",
        llhttp_method_name(request->method),
        request->url,
        request->version.major,
        request->version.minor
    );
}

static void log_response(http_response_t *response) {
    log_info(
        "response: HTTP/%d.%d %d %s",
        response->version.major,
        response->version.minor,
        response->status,
        llhttp_status_name(response->status)
    );
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
            return ERROR;
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
                return ERROR;
            }

            sent += n;
        }

        buffer_destroy(buffer);
    }

    return SUCCESS;
}

static int
recv_request(http_request_t *request, void **buffer, int *len, int client) {
    int request_bufsize = REQUEST_BUFSIZE;
    void *request_buffer = malloc(request_bufsize);

    int rcvd = 0;

    while (!request->finished) {
        if (rcvd == request_bufsize) {
            request_bufsize *= 2;
            request_buffer = realloc(request_buffer, request_bufsize);
        }

        int n = read(client, request_buffer + rcvd, request_bufsize - rcvd);
        if (n <= 0) {
            free(request_buffer);
            return ERROR;
        }

        int err = http_request_parse(request, request_buffer + rcvd, n);
        if (err) {
            log_error("error parsing request");
            free(request_buffer);
            return ERROR;
        }

        rcvd += n;
    }

    *buffer = request_buffer;
    *len = rcvd;

    return SUCCESS;
}

static int connect_from_url(char *url) {
    char *host = http_host_from_url(url);
    if (!host) {
        return ERROR;
    }

    int remote = net_connect_remote(host, "http");
    if (remote < 0) {
        free(host);
        return ERROR;
    }

    free(host);
    return remote;
}

static int create_detached_thread(void *(*start)(void *), void *arg) {
    int err;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t tid;

    err = pthread_create(&tid, &attr, start, arg);
    if (err) {
        log_error("error creating thread: %s", strerror(err));
        pthread_attr_destroy(&attr);
        return ERROR;
    }

    pthread_attr_destroy(&attr);
    return SUCCESS;
}

int client_handler(int client, cache_t *cache) {
    int err;
    void *request_buffer;
    int rcvd;
    http_request_t request;
    http_request_init(&request);

    err = recv_request(&request, &request_buffer, &rcvd, client);
    if (err != SUCCESS) {
        close(client);
        return ERROR;
    }

    log_request(&request);

    if (!is_allowed_request(&request)) {
        log_warn("rejected request with wrong version or method");
        free(request_buffer);
        close(client);
        return ERROR;
    }

    stream_t *cached;
    stream_t *inserted = stream_create(RESPONSE_BUFSIZE);
    sieve_cache_lookup_or_insert(cache, request.url, &cached, inserted);

    if (cached) {
        free(request_buffer);
        stream_destroy(inserted);
        err = send_from_cache(client, cached);
        close(client);
        stream_destroy(cached);
        return err;
    }

    int remote = connect_from_url(request.url);
    if (remote < 0) {
        stream_signal_error(inserted);
        stream_destroy(inserted);
        free(request_buffer);
        close(client);
        return ERROR;
    }

    int sent = 0;
    while (sent < rcvd) {
        int n = write(remote, request_buffer + sent, rcvd - sent);

        if (n <= 0) {
            stream_signal_error(inserted);
            stream_destroy(inserted);
            free(request_buffer);
            close(client);
            close(remote);
            return ERROR;
        }

        sent += n;
    }

    free(request_buffer);

    server_ctx_t *ctx = malloc(sizeof(server_ctx_t));
    ctx->remote = remote;
    ctx->stream = stream_clone(inserted);

    err = create_detached_thread(server_thread, ctx);
    if (err != SUCCESS) {
        stream_signal_error(inserted);
        stream_destroy(inserted);
        stream_destroy(inserted);
        free(ctx);
        return ERROR;
    }

    err = send_from_cache(client, inserted);
    close(client);
    stream_destroy(inserted);
    return err;
}

int server_handler(int remote, stream_t *stream) {
    int err;

    http_response_t response;
    http_response_init(&response);

    while (1) {
        pthread_mutex_lock(&stream->mutex);

        if (response.finished) {
            pthread_mutex_unlock(&stream->mutex);
            break;
        }

        if (stream->buffer->cap == stream->len) {
            buffer_t *old = stream->buffer;
            buffer_t *new = buffer_create(old->cap * 2);
            memcpy(&new->buf, &old->buf, stream->len);
            stream->buffer = new;
            buffer_destroy(old);
        }

        buffer_t *stream_buffer = stream->buffer;
        int stream_len = stream->len;

        pthread_mutex_unlock(&stream->mutex);

        int n = read(
            remote,
            stream_buffer->buf + stream_len,
            stream_buffer->cap - stream_len
        );

        if (n > 0) {
            err = http_response_parse(
                &response, stream_buffer->buf + stream_len, n
            );

            if (err) {
                log_error("error parsing response");
            }
        }

        pthread_mutex_lock(&stream->mutex);

        int ret = SUCCESS;

        if (n <= 0 || err == -1) {
            stream->erred = true;
            ret = ERROR;
        }

        stream->len += n;

        if (response.finished) {
            stream->complete = true;
        }

        pthread_cond_broadcast(&stream->cond);
        pthread_mutex_unlock(&stream->mutex);

        if (ret != SUCCESS || atomic_load(&stream->refcount) == 1) {
            close(remote);
            stream_destroy(stream);
            return ret;
        }
    }

    log_response(&response);

    close(remote);
    stream_destroy(stream);
    return SUCCESS;
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
