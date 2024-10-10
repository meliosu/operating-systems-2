#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http.h"
#include "log.h"
#include "logic.h"
#include "net.h"
#include "sieve.h"
#include "stream.h"

#define BUFSIZE_REQUEST (128 * 1024)
#define BUFSIZE_RESPONSE (128 * 1024)

void *client_thread(void *arg) {
    struct client_ctx *ctx = arg;

    int err = client_handler(ctx->cache, ctx->client);
    if (err) {
        ERROR("error handling client");
    }

    free(ctx);
    return NULL;
}

void *server_thread(void *arg) {
    struct server_ctx *ctx = arg;

    int err = server_handler(ctx->stream, ctx->server);
    if (err) {
        ERROR("error handling server");
    }

    free(ctx);
    return NULL;
}

int client_send_cached(struct stream *stream, int client) {
    int bytes;
    int size = 0;
    int sent = 0;

    while (1) {
        pthread_mutex_lock(&stream->mutex);

        if (stream->erred) {
            pthread_mutex_unlock(&stream->mutex);
            return -1;
        }

        size = stream->len;

        if (size == sent && stream->complete) {
            pthread_mutex_unlock(&stream->mutex);
            break;
        } else if (size == sent) {
            pthread_cond_wait(&stream->cond, &stream->mutex);
        } else {
            pthread_mutex_unlock(&stream->mutex);

            bytes = write(client, stream->data + sent, size - sent);
            if (bytes <= 0) {
                return -1;
            }

            sent += bytes;
        }
    }

    return 0;
}

int client_handler(struct cache *cache, int client) {
    int sent, rcvd, bytes, err;

    struct http_request request;
    http_request_init(&request);

    struct http_response response;
    http_response_init(&response);

    char *request_buffer = malloc(BUFSIZE_REQUEST);

    rcvd = 0;
    bytes = read(client, request_buffer, BUFSIZE_REQUEST);
    if (bytes <= 0) {
        return -1;
    }

    rcvd += bytes;
    err = http_request_parse(&request, request_buffer, rcvd);
    if (err == -1) {
        ERROR("error parsing request");
        return -1;
    }

    if (request.method != HTTP_GET ||
        !(request.version.major == 1 && request.version.minor == 0)) {
        ERROR("wrong version or method");
        return -1;
    }

    while (!request.finished) {
        bytes = read(client, request_buffer + rcvd, BUFSIZE_REQUEST - rcvd);
        if (bytes <= 0) {
            return -1;
        }

        rcvd += bytes;
        err = http_request_parse(&request, request_buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing request");
            return -1;
        }
    }

    struct stream *stream;
    sieve_cache_lookup(cache, request.url, (void **)&stream);

    if (stream) {
        err = client_send_cached(stream, client);
        if (err) {
            return -1;
        }
    } else {
        char *host = http_host_from_url(request.url);
        if (!host) {
            ERROR("error getting host from url");
            return -1;
        }

        int server = net_connect_remote(host, "80");
        if (server < 0) {
            ERROR("error connecting to remote");
            return -1;
        }

        sent = 0;
        while (sent < rcvd) {
            bytes = write(server, request_buffer + sent, rcvd - sent);
            if (bytes <= 0) {
                return -1;
            }

            sent += bytes;
        }

        char *response_buffer = malloc(BUFSIZE_RESPONSE);

        rcvd = 0;
        sent = 0;
        bytes = read(server, response_buffer, BUFSIZE_RESPONSE);
        if (bytes <= 0) {
            return -1;
        }

        rcvd += bytes;
        err = http_response_parse(&response, response_buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing response");
            return -1;
        }

        if (response.status != 200) {
            while (sent < rcvd || !response.finished) {
                bytes = write(client, response_buffer + sent, rcvd - sent);
                if (bytes <= 0) {
                    return -1;
                }

                if (response.finished) {
                    continue;
                }

                sent += bytes;
                bytes = read(
                    server, response_buffer + rcvd, BUFSIZE_RESPONSE - rcvd
                );
                if (bytes <= 0) {
                    return -1;
                }

                rcvd += bytes;
                err = http_response_parse(&response, response_buffer, rcvd);
                if (err == -1) {
                    ERROR("error parsing response");
                    return -1;
                }
            }
        } else {
            struct stream *looked_up;
            struct stream *inserted = malloc(sizeof(struct stream));

            stream_init(inserted);
            inserted->data = response_buffer;
            inserted->len = rcvd;

            sieve_cache_lookup_or_insert(
                cache, request.url, (void **)&looked_up, inserted
            );

            if (looked_up) {
                stream_destroy(inserted);
                close(server);
                err = client_send_cached(looked_up, client);
                if (err) {
                    return -1;
                }
            } else {
                struct server_ctx *ctx = malloc(sizeof(*ctx));

                ctx->stream = inserted;
                ctx->server = server;

                pthread_attr_t attr;
                pthread_t tid;

                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

                err = pthread_create(&tid, &attr, server_thread, ctx);
                if (err) {
                    ERROR("pthread_create: %s\n", strerror(-err));
                    return -1;
                }

                pthread_attr_destroy(&attr);

                err = client_send_cached(inserted, client);
                if (err) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

int server_handler(struct stream *stream, int server) {
    int bytes, err;

    struct http_response response;
    http_response_init(&response);

    while (!response.finished) {
        bytes = read(
            server, stream->data + stream->len, BUFSIZE_RESPONSE - stream->len
        );

        if (bytes <= 0) {
            pthread_mutex_lock(&stream->mutex);

            stream->erred = true;

            pthread_mutex_unlock(&stream->mutex);
            pthread_cond_broadcast(&stream->cond);
            return -1;
        }

        err = http_response_parse(&response, stream->data, stream->len + bytes);
        pthread_mutex_lock(&stream->mutex);

        if (err == -1) {
            stream->erred = true;
        } else if (err == 0) {
            stream->len += bytes;
            stream->complete = true;
        } else {
            stream->len += bytes;
        }

        pthread_mutex_unlock(&stream->mutex);
        pthread_cond_broadcast(&stream->cond);

        if (err == -1) {
            return -1;
        } else if (err == 0) {
            break;
        }
    }

    return 0;
}
