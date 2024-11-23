#ifndef PROXY_HTTP_H
#define PROXY_HTTP_H

#include <llhttp.h>

typedef struct http_request {
    struct {
        int major;
        int minor;
    } version;
    int method;
    char *url;

    int finished;
    llhttp_t parser;
    llhttp_settings_t settings;
} http_request_t;

typedef struct http_response {
    struct {
        int major;
        int minor;
    } version;
    int status;

    int finished;
    llhttp_t parser;
    llhttp_settings_t settings;
} http_response_t;

void http_request_init(http_request_t *request);
void http_response_init(http_response_t *response);

int http_request_parse(http_request_t *request, char *buf, int len);
int http_response_parse(http_response_t *response, char *buf, int len);

char *http_host_from_url(char *url);

#endif /* PROXY_HTTP_H */
