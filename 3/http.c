#include <stdio.h>
#include <stdlib.h>

#include "http.h"

void http_request_init(struct http_request *request) {
    request->version.major = 0;
    request->version.minor = 0;
    request->method = NULL;
    request->url = NULL;
}

void http_request_destroy(struct http_request *request) {
    free(request->method);
    free(request->url);
}

void http_response_init(struct http_response *response) {
    response->version.major = 0;
    response->version.minor = 0;
    response->status = 0;
    response->phrase = NULL;
}

void http_response_destroy(struct http_response *response) {
    free(response->phrase);
}

int http_request_parse(struct http_request *request, char *buffer, int len) {
    // TODO: handle incomplete input

    int matched = sscanf(
        buffer,
        "%ms %ms HTTP/%d.%d\r\n",
        &request->method,
        &request->url,
        &request->version.major,
        &request->version.minor
    );

    if (matched < 4) {
        // TODO: free memory allocated for strings

        return -1;
    }

    return 0;
}

int http_response_parse(struct http_response *response, char *buffer, int len) {
    // TODO: handle incomplete input

    int matched = sscanf(
        buffer,
        "HTTP/%d.%d %d %m[^\r\n]\r\n",
        &response->version.major,
        &response->version.minor,
        &response->status,
        &response->phrase
    );

    if (matched < 4) {
        // TODO: free memory allocated for strings

        return -1;
    }

    return 0;
}
