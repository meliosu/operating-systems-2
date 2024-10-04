#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <llhttp.h>

#include "http.h"
#include "log.h"
#include "net.h"
#include "threads.h"

#define REQUEST_BUFSIZE (64 * 1024)
#define RESPONSE_BUFSIZE (256 * 1024)

static void proxy_uncached(
    struct http_request *request, int clientfd, void *request_buffer, int len
) {
    char *host = http_host_from_url(request->url);
    if (!host) {
        ERROR("failed to get host from url");
        return;
    }

    TRACE("connecting...");
    int serverfd = net_connect_remote(host, "80");
    if (serverfd < 0) {
        ERROR("failed to connect to remote host");
        return;
    }

    TRACE("connected");

    int rcvd = len;
    int sent = 0;
    while (sent < rcvd || !request->finished) {
        TRACE("sending to server...");
        int n = write(serverfd, request_buffer + sent, rcvd - sent);
        TRACE("sent");
        if (n < 0) {
            ERROR("error writing to server socket");
        }

        sent += n;

        if (!request->finished) {
            int m =
                read(clientfd, request_buffer + rcvd, REQUEST_BUFSIZE - rcvd);
            if (m < 0) {
                ERROR("error reading from client socket");
            }

            rcvd += m;
        }

        int err = http_request_parse(request, request_buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing HTTP request");
        }
    }

    TRACE("after receiving request, finished: %d", request->finished);
    TRACE("request: %.*s", rcvd, (char *)request_buffer);
    printf("%.*s", rcvd, (char *)request_buffer);

    TRACE("sent: %d, rcvd: %d", sent, rcvd);

    free(request_buffer);

    // by this point we have sent request to the server

    struct http_response response;
    http_response_init(&response);

    void *response_buffer = malloc(RESPONSE_BUFSIZE);

    rcvd = 0;
    sent = 0;

    while ((sent < rcvd || !response.finished) || (sent == 0 && rcvd == 0)) {
        TRACE("reading from server...");
        int n = read(serverfd, response_buffer + rcvd, RESPONSE_BUFSIZE - rcvd);
        if (n < 0) {
            ERROR("error reading from server socket");
        }

        TRACE("read %d bytes", n);

        rcvd += n;

        TRACE("writing to client");
        int m = write(clientfd, response_buffer + sent, rcvd - sent);
        if (m < 0) {
            ERROR("error writing to client socket");
        }

        TRACE("written %d bytes", m);

        sent += m;

        TRACE("response: %.*s", rcvd, (char *)response_buffer);

        int err = http_response_parse(&response, response_buffer, rcvd);
        if (err == -1) {
            ERROR("error parsing HTTP response");
        }
    }

    TRACE("closing");
    close(clientfd);
    close(serverfd);
}

void *thread_clientside(void *arg) {
    struct clientside_ctx *context = arg;

    struct http_request request;
    http_request_init(&request);

    void *buffer = malloc(REQUEST_BUFSIZE);
    int received = read(context->clientfd, buffer, REQUEST_BUFSIZE);
    if (received < 0) {
        ERROR("error reading from client socket");
    }

    http_request_parse(&request, buffer, received);

    /*if (request.method != HTTP_GET) {*/
    proxy_uncached(&request, context->clientfd, buffer, received);
    /*}*/

    return NULL;
}

void *thread_serverside(void *arg) {
    return NULL;
}
