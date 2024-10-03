#ifndef PROXY_HTTP_H
#define PROXY_HTTP_H

#include "slice.h"

#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_HEAD 3
#define METHOD_OTHER 4

#define http_headers_foreach(headers, curr)                                    \
    for (struct http_header *curr = headers.first; curr != NULL;               \
         curr = curr->next)

struct http_header {
    slice_t key;
    slice_t value;

    struct http_header *next;
};

struct http_headers {
    struct http_header *first;
    struct http_header *last;
};

struct http_version {
    int major;
    int minor;
};

struct http_request {
    char *path;
    int method;
    slice_t body;
    struct http_version version;
    struct http_headers headers;
};

struct http_response {
    int status;
    slice_t phrase;
    slice_t body;
    struct http_version version;
    struct http_headers headers;
};

struct http_cursor {
    int pos;
    int state;
};

void http_cursor_init(struct http_cursor *cursor);

void http_headers_init(struct http_headers *headers);
void http_headers_destroy(struct http_headers *headers);

void http_request_init(struct http_request *request);
void http_request_destroy(struct http_request *request);

void http_response_init(struct http_response *response);
void http_response_destroy(struct http_response *response);

struct http_header *http_header_find(struct http_headers *hdrs, char *key);

int http_path_get_name(char *path, char **name);

int http_request_parse_status(
    struct http_request *request,
    struct http_cursor *cursor,
    char *buffer,
    int len
);

int http_response_parse_status(
    struct http_response *response,
    struct http_cursor *cursor,
    char *buffer,
    int len
);

int http_request_parse(
    struct http_request *request,
    struct http_cursor *cursor,
    char *buffer,
    int len
);

int http_response_parse(
    struct http_response *response,
    struct http_cursor *cursor,
    char *buffer,
    int len
);

#endif /* PROXY_HTTP_H */
