#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <llhttp.h>

#include "http.h"
#include "log.h"
#include "net.h"
#include "sieve.h"
#include "stream.h"
#include "threads.h"

#define REQUEST_BUFSIZE (64 * 1024)
#define RESPONSE_BUFSIZE (256 * 1024)

static int connect_to(char *url) {
    char *host = http_host_from_url(url);
    if (!host) {
        return -1;
    }

    int serverfd = net_connect_remote(host, "80");
    if (serverfd < 0) {
        return -1;
    }

    return serverfd;
}

static int send_request(
    struct http_request *request, int clientfd, char *buffer, int rcvd
) {
    int serverfd = connect_to(request->url);
    if (serverfd < 0) {
        return -1;
    }

    int sent = 0;
    while (sent < rcvd || !request->finished) {
        int n = write(serverfd, buffer + sent, rcvd - sent);
        if (n < 0) {
            ERROR("error writing to server socket");
        }

        sent += n;

        if (request->finished) {
            continue;
        }

        int m = read(clientfd, buffer + rcvd, REQUEST_BUFSIZE - rcvd);
        if (m < 0) {
            ERROR("error reading from client socket");
        }

        rcvd += m;

        int err = http_request_parse(request, buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing http request");
        }
    }

    return serverfd;
}

static int recv_response(
    struct http_response *response,
    int clientfd,
    int serverfd,
    char *buffer,
    int rcvd
) {
    int sent = 0;

    while (sent < rcvd || !response->finished) {
        if (!response->finished) {
            int n = read(serverfd, buffer + rcvd, RESPONSE_BUFSIZE - rcvd);
            if (n < 0) {
                ERROR("error reading from server socket");
                return -1;
            }

            rcvd += n;

            int err = http_response_parse(response, buffer, rcvd);
            if (err == -1) {
                ERROR("error parsing HTTP response");
                return -1;
            }
        }

        int m = write(clientfd, buffer + sent, rcvd - sent);
        if (m < 0) {
            ERROR("error writing to client socket");
            return -1;
        }

        sent += m;
    }

    return 0;
}

static void proxy_uncached(
    struct http_request *request, int clientfd, void *request_buffer, int len
) {
    int serverfd = connect_to(request->url);
    if (serverfd < 0) {
        ERROR("error connecting");
        return;
    }

    int rcvd = len;
    int sent = 0;
    while (sent < rcvd || !request->finished) {
        int n = write(serverfd, request_buffer + sent, rcvd - sent);
        if (n < 0) {
            ERROR("error writing to server socket");
            goto exit;
        }

        sent += n;

        if (request->finished) {
            continue;
        }

        int m = read(clientfd, request_buffer + rcvd, REQUEST_BUFSIZE - rcvd);
        if (m < 0) {
            ERROR("error reading from client socket");
            goto exit;
        }

        rcvd += m;

        int err = http_request_parse(request, request_buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing HTTP request");
            goto exit;
        }
    }

    free(request_buffer);

    struct http_response response;
    http_response_init(&response);

    void *response_buffer = malloc(RESPONSE_BUFSIZE);

    rcvd = 0;
    sent = 0;

    while ((sent < rcvd || !response.finished) || (sent == 0 && rcvd == 0)) {
        if (!response.finished) {
            int n =
                read(serverfd, response_buffer + rcvd, RESPONSE_BUFSIZE - rcvd);
            if (n < 0) {
                ERROR("error reading from server socket");
                goto exit;
            }

            rcvd += n;

            int err = http_response_parse(&response, response_buffer, rcvd);
            if (err == -1) {
                ERROR("error parsing HTTP response");
                goto exit;
            }
        }

        int m = write(clientfd, response_buffer + sent, rcvd - sent);
        if (m < 0) {
            ERROR("error writing to client socket");
            goto exit;
        }

        sent += m;
    }

exit:
    close(clientfd);
    close(serverfd);
}

int send_cached_response(struct stream *stream, int clientfd) {
    char *buffer;
    int sent = 0;
    int size;
    while (1) {
        pthread_mutex_lock(&stream->mutex);

        buffer = stream->data;
        size = stream->len;

        if (size == sent && stream->complete) {
            pthread_mutex_unlock(&stream->mutex);
            break;
        }

        if (size == sent) {
            pthread_cond_wait(&stream->cond, &stream->mutex);
        } else {
            pthread_mutex_unlock(&stream->mutex);

            int n = write(clientfd, buffer + sent, size - sent);
            if (n < 0) {
                ERROR("error writing to client socket");
            }
        }
    }

    return 0;
}

int load_response_into_cache(
    struct stream *stream,
    struct http_response *response,
    char *buffer,
    int rcvd,
    int serverfd
) {
    while (1) {
        int n = read(serverfd, buffer + rcvd, RESPONSE_BUFSIZE - rcvd);
        if (n < 0) {
            ERROR("error reading while loading into cache");
            break;
        }

        rcvd += n;

        int err = http_response_parse(response, buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing response");
            break;
        }

        pthread_mutex_lock(&stream->mutex);

        stream->len = rcvd;

        if (response->finished) {
            stream->complete = 1;
        }

        pthread_mutex_unlock(&stream->mutex);
        pthread_cond_broadcast(&stream->cond);

        if (response->finished) {
            break;
        }
    }

    return 0;
}

void *thread_clientside(void *arg) {
    struct clientside_ctx *context = arg;

    struct http_request request;
    http_request_init(&request);

    void *buffer = malloc(REQUEST_BUFSIZE);

    int received = read(context->clientfd, buffer, REQUEST_BUFSIZE);
    if (received < 0) {
        ERROR("error reading from client socket");
        return NULL;
    }

    if (received == 0) {
        return NULL;
    }

    int err = http_request_parse(&request, buffer, received);
    if (err == -1) {
        ERROR("error parsing http request");
        return NULL;
    }

    if (!(request.version.major == 1 && request.version.minor == 0)) {
        INFO("rejected client with wrong http version");
        return NULL;
    }

    INFO(
        "new request: %s %s %d.%d",
        llhttp_method_name(request.method),
        request.url,
        request.version.major,
        request.version.minor
    );

    if (request.method != HTTP_GET) {
        proxy_uncached(&request, context->clientfd, buffer, received);
        return NULL;
    }

    struct stream *stream;
    sieve_cache_lookup(context->cache, request.url, (void **)&stream);

    if (stream) {
        send_cached_response(stream, context->clientfd);
        return NULL;
    }

    int server = send_request(&request, context->clientfd, buffer, received);
    if (server == -1) {
        ERROR("error sending request");
        return NULL;
    }

    char *response_buffer = malloc(RESPONSE_BUFSIZE);

    struct http_response response;
    http_response_init(&response);

    int rcvd = read(server, response_buffer, RESPONSE_BUFSIZE);
    if (rcvd < 0) {
        ERROR("error reading from server");
    }

    if (response.status != 200) {
        err = recv_response(
            &response, context->clientfd, server, response_buffer, rcvd
        );

        if (err) {
            ERROR("error receiving response");
        }

        return NULL;
    }

    stream = malloc(sizeof(struct stream));
    stream_init(stream);

    stream->data = buffer;
    stream->len = rcvd;
    stream->complete = response.finished;

    struct serverside_ctx *ctx = malloc(sizeof(struct serverside_ctx));
    ctx->serverfd = server;
    ctx->stream = stream;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t tid;
    err = pthread_create(&tid, &attr, thread_serverside, ctx);
    if (err) {
        ERROR("error creating thread");
        return NULL;
    }

    err = send_cached_response(stream, context->clientfd);
    if (err) {
        ERROR("error sending cached response");
        return NULL;
    }

    return NULL;
}

void *thread_serverside(void *arg) {
    TRACE("serverside thread");

    struct serverside_ctx *ctx = arg;
    int err;

    struct http_response response;
    http_response_init(&response);

    http_response_parse(&response, ctx->stream->data, ctx->stream->len);

    err = load_response_into_cache(
        ctx->stream,
        &response,
        ctx->stream->data,
        ctx->stream->len,
        ctx->serverfd
    );

    if (err) {
        ERROR("error loading response into cache");
    }

    return NULL;
}
