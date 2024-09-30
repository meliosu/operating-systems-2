#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http.h"

#define STATE_STATUSLINE 0
#define STATE_HEADER 1
#define STATE_BODY 2
#define STATE_FINISHED 3

void http_headers_init(struct http_headers *headers) {
    headers->first = NULL;
    headers->last = NULL;
}

void http_headers_destroy(struct http_headers *headers) {
    struct http_header *curr;

    while (curr) {
        struct http_header *next = curr->next;
        free(curr);
        curr = next;
    }
}

void http_headers_add(
    struct http_headers *headers, struct http_header *header
) {
    struct http_header *header_heap = malloc(sizeof(*header_heap));
    header_heap->key = header->key;
    header_heap->value = header->value;

    if (!headers->first) {
        headers->first = headers->last = header_heap;
    } else {
        headers->last = headers->last->next = header_heap;
    }
}

void http_request_init(struct http_request *request) {
    memset(request, 0, sizeof(*request));
}

void http_response_init(struct http_response *response) {
    memset(response, 0, sizeof(*response));
}

void http_request_destroy(struct http_request *request) {
    http_headers_destroy(&request->headers);
}

void http_response_destroy(struct http_response *response) {
    http_headers_destroy(&response->headers);
}

void http_cursor_init(struct http_cursor *cursor) {
    cursor->pos = 0;
    cursor->state = STATE_STATUSLINE;
}

void http_cursor_copy(struct http_cursor *out, struct http_cursor *in) {
    out->pos = in->pos;
    out->state = in->state;
}

int http_version_parse(
    struct http_version *version,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    int maj;
    int min;
    int end;

    if (sscanf(buffer + cursor->pos, "HTTP/%d.%d%n", &maj, &min, &end) < 2) {
        return -1;
    }

    version->major = maj;
    version->minor = min;
    cursor->pos += end;
    return 0;
}

int http_method_parse(
    int *method, struct http_cursor *cursor, char *buffer, int len
) {
    int end;

    if (sscanf(buffer + cursor->pos, "%*s%n", &end) == -1) {
        return -1;
    }

    if (!strncmp(buffer + cursor->pos, "GET", end)) {
        *method = METHOD_GET;
    } else if (!strncmp(buffer + cursor->pos, "POST", end)) {
        *method = METHOD_POST;
    } else if (!strncmp(buffer + cursor->pos, "HEAD", end)) {
        *method = METHOD_HEAD;
    } else {
        *method = METHOD_OTHER;
    }

    cursor->pos += end;
    return 0;
}

int http_status_parse(
    int *status, struct http_cursor *cursor, char *buffer, int len
) {
    int res;
    int end;

    if (sscanf(buffer + cursor->pos, "%d%n", &res, &end) < 1) {
        return -1;
    }

    *status = res;
    cursor->pos += end;
    return 0;
}

int http_phrase_parse(
    slice_t *phrase, struct http_cursor *cursor, char *buffer, int len
) {
    int end;

    if (sscanf(buffer + cursor->pos, "%*[^\r\n]%n", &end) == -1) {
        return -1;
    }

    phrase->ptr = buffer + cursor->pos;
    phrase->len = end;
    cursor->pos += end;
    return 0;
}

int http_parse_path(
    slice_t *path, struct http_cursor *cursor, char *buffer, int len
) {
    int end;

    if (sscanf(buffer + cursor->pos, "%*s%n", &end) == -1) {
        return -1;
    }

    path->ptr = buffer + cursor->pos;
    path->len = end;
    cursor->pos += end;
    return 0;
}

int http_parse_space(struct http_cursor *cursor, char *buffer, int len) {
    if (sscanf(buffer + cursor->pos, " ") == -1) {
        return -1;
    }

    cursor->pos += 1;
    return 0;
}

int http_parse_crlf(struct http_cursor *cursor, char *buffer, int len) {
    if (sscanf(buffer + cursor->pos, "\r\n") == -1) {
        return -1;
    }

    cursor->pos += 2;
    return 0;
}

int http_request_parse_status(
    struct http_request *request,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    if (!memmem(buffer, len, "\r\n", 2)) {
        return -2;
    }

    struct http_cursor c;
    http_cursor_copy(&c, cursor);

    int method;
    if (http_method_parse(&method, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_space(&c, buffer, len) < 0) {
        return -1;
    }

    slice_t path;
    if (http_parse_path(&path, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_space(&c, buffer, len) < 0) {
        return -1;
    }

    struct http_version version;
    if (http_version_parse(&version, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_crlf(&c, buffer, len) < 0) {
        return -1;
    }

    request->method = method;
    request->path = path;
    request->version = version;
    http_cursor_copy(cursor, &c);
    return 0;
}

int http_response_parse_status(
    struct http_response *response,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    if (!memmem(buffer, len, "\r\n", 2)) {
        return -2;
    }

    struct http_cursor c;
    http_cursor_copy(&c, cursor);

    struct http_version version;
    if (http_version_parse(&version, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_space(&c, buffer, len) < 0) {
        return -1;
    }

    int status;
    if (http_status_parse(&status, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_space(&c, buffer, len) < 0) {
        return -1;
    }

    slice_t phrase;
    if (http_phrase_parse(&phrase, &c, buffer, len) < 0) {
        return -1;
    }

    if (http_parse_crlf(&c, buffer, len) < 0) {
        return -1;
    }

    response->version = version;
    response->status = status;
    response->phrase = phrase;
    http_cursor_copy(cursor, &c);
    return 0;
}

int http_header_parse(
    struct http_header *header,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    if (!memmem(buffer + cursor->pos, len - cursor->pos, "\r\n", 2)) {
        return -2;
    }

    int key_end;
    int value_end;
    int matched;

    matched = sscanf(
        buffer + cursor->pos, "%*[^:]%n: %*s%n\r\n", &key_end, &value_end
    );

    if (matched <= 0) {
        return -1;
    } else if (matched == 1) {
        header->key.ptr = buffer + cursor->pos;
        header->key.len = key_end;
    } else {
        header->key.ptr = buffer + cursor->pos;
        header->key.len = key_end;
        header->value.ptr = buffer + key_end + 2;
        header->value.len = value_end - key_end - 2;
    }

    cursor->pos += value_end + 2;
    return 0;
}

struct http_header *http_header_find(struct http_headers *hdrs, char *hdr) {
    struct http_header *curr = hdrs->first;

    while (curr) {
        if (!strncmp(curr->key.ptr, hdr, curr->key.len)) {
            break;
        }
    }

    return curr;
}

long http_body_length(struct http_headers *headers) {
    long len = -1;
    struct http_header *c_length;

    if ((c_length = http_header_find(headers, "Content-Length"))) {
        char *end;
        len = strtol(c_length->value.ptr, &end, c_length->value.len);

        if (end - c_length->value.ptr != c_length->value.len) {
            len = -1;
        }
    }

    return len;
}

int http_request_parse(
    struct http_request *request,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    int ret;
    int body_len;
    struct http_header header;

    while (cursor->state != STATE_FINISHED) {
        switch (cursor->state) {
        case STATE_STATUSLINE:
            ret = http_request_parse_status(request, cursor, buffer, len);

            if (ret < 0) {
                return ret;
            }

            cursor->state = STATE_HEADER;
            continue;

        case STATE_HEADER:
            if (!http_parse_crlf(cursor, buffer, len)) {
                body_len = http_body_length(&request->headers);

                if (body_len <= 0) {
                    cursor->state = STATE_FINISHED;
                } else {
                    request->body.ptr = buffer + cursor->pos;
                    request->body.len = body_len;
                    cursor->state = STATE_BODY;
                }

                continue;
            }

            ret = http_header_parse(&header, cursor, buffer, len);
            if (ret < 0) {
                return ret;
            }

            http_headers_add(&request->headers, &header);
            continue;

        case STATE_BODY:
            if (buffer + len >= request->body.ptr + request->body.len) {
                cursor->pos = request->body.ptr - buffer + request->body.len;
                cursor->state = STATE_FINISHED;
            } else {
                cursor->pos = len;
            }

            continue;
        }
    }

    return cursor->pos;
}

int http_response_parse(
    struct http_response *response,
    struct http_cursor *cursor,
    char *buffer,
    int len
) {
    int ret;
    int body_len;
    struct http_header header;

    while (cursor->state != STATE_FINISHED) {
        switch (cursor->state) {
        case STATE_STATUSLINE:
            ret = http_response_parse_status(response, cursor, buffer, len);

            if (ret < 0) {
                return ret;
            }

            cursor->state = STATE_HEADER;
            continue;

        case STATE_HEADER:
            if (!http_parse_crlf(cursor, buffer, len)) {
                body_len = http_body_length(&response->headers);

                if (body_len <= 0) {
                    cursor->state = STATE_FINISHED;
                } else {
                    response->body.ptr = buffer + cursor->pos;
                    response->body.len = body_len;
                    cursor->state = STATE_BODY;
                }

                continue;
            }

            ret = http_header_parse(&header, cursor, buffer, len);
            if (ret < 0) {
                return ret;
            }

            http_headers_add(&response->headers, &header);
            continue;

        case STATE_BODY:
            if (buffer + len >= response->body.ptr + response->body.len) {
                cursor->pos = response->body.ptr - buffer + response->body.len;
                cursor->state = STATE_FINISHED;
            } else {
                cursor->pos = len;
            }

            continue;
        }
    }

    return cursor->pos;
}
