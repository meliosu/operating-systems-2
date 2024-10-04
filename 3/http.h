#ifndef PROXY_HTTP_H
#define PROXY_HTTP_H

#include <llhttp.h>

struct http_request {
    struct {
        int major;
        int minor;
    } version;
    int method;
    char *url;

    int finished;
    llhttp_t parser;
    llhttp_settings_t settings;
};

struct http_response {
    struct {
        int major;
        int minor;
    } version;
    int status;

    int finished;
    llhttp_t parser;
    llhttp_settings_t settings;
};

void http_request_init(struct http_request *request);
void http_response_init(struct http_response *response);

int http_request_parse(struct http_request *request, char *buf, int len);
int http_response_parse(struct http_response *response, char *buf, int len);

char *http_host_from_url(char *url);

#endif /* PROXY_HTTP_H */
