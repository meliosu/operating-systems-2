#include <stdio.h>

#include "http.h"

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
        return -1;
    }

    return 0;
}
