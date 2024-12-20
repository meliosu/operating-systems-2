#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <llhttp.h>

#include "http.h"

static int on_method_complete(llhttp_t *parser) {
    http_request_t *request = parser->data;
    request->method = llhttp_get_method(parser);
    return 0;
}

static int on_url(llhttp_t *parser, const char *url, unsigned long len) {
    http_request_t *request = parser->data;

    if (request->url) {
        int curr_len = strlen(request->url);
        request->url = realloc(request->url, curr_len + len + 1);
        memcpy(request->url + curr_len, url, len);
        request->url[curr_len + len] = 0;
    } else {
        request->url = strndup(url, len);
    }

    return 0;
}

static int on_version_complete_request(llhttp_t *parser) {
    http_request_t *request = parser->data;
    request->version.major = llhttp_get_http_major(parser);
    request->version.minor = llhttp_get_http_minor(parser);
    return 0;
}

static int on_version_complete_response(llhttp_t *parser) {
    http_response_t *response = parser->data;
    response->version.major = llhttp_get_http_major(parser);
    response->version.minor = llhttp_get_http_minor(parser);
    return 0;
}

static int on_request_complete(llhttp_t *parser) {
    http_request_t *request = parser->data;
    request->finished = 1;
    return 0;
}

static int on_response_complete(llhttp_t *parser) {
    http_response_t *response = parser->data;
    response->finished = 1;
    return 0;
}

static int on_status_complete(llhttp_t *parser) {
    http_response_t *response = parser->data;
    response->status = llhttp_get_status_code(parser);
    return 0;
}

void http_request_init(http_request_t *request) {
    memset(request, 0, sizeof(http_request_t));

    llhttp_settings_init(&request->settings);
    request->settings.on_message_complete = on_request_complete;
    request->settings.on_version_complete = on_version_complete_request;
    request->settings.on_url = on_url;
    request->settings.on_method_complete = on_method_complete;

    llhttp_init(&request->parser, HTTP_REQUEST, &request->settings);
    request->parser.data = request;
}

void http_response_init(http_response_t *response) {
    memset(response, 0, sizeof(http_response_t));

    llhttp_settings_init(&response->settings);
    response->settings.on_message_complete = on_response_complete;
    response->settings.on_version_complete = on_version_complete_response;
    response->settings.on_status_complete = on_status_complete;

    llhttp_init(&response->parser, HTTP_RESPONSE, &response->settings);
    response->parser.data = response;
}

int http_request_parse(http_request_t *request, char *buf, int len) {
    llhttp_errno_t err = llhttp_execute(&request->parser, buf, len);
    if (err != HPE_OK) {
        return -1;
    }

    return 0;
}

int http_response_parse(http_response_t *response, char *buf, int len) {
    llhttp_errno_t err = llhttp_execute(&response->parser, buf, len);
    if (err != HPE_OK) {
        return -1;
    }

    return 0;
}

char *http_host_from_url(char *url) {
    char *host;
    if (!url) {
        return NULL;
    }

    if (sscanf(url, "http://%m[^/]%*s", &host) < 1) {
        return NULL;
    }

    return host;
}
