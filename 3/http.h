#ifndef PROXY_HTTP_H
#define PROXY_HTTP_H

struct http_request {
    char *method;
    char *url;
    struct {
        int major;
        int minor;
    } version;
};

struct http_response {
    int status;
    char *phrase;
    struct {
        int major;
        int minor;
    } version;
};

int http_request_parse(struct http_request *request, char *buffer, int len);
int http_response_parse(struct http_response *response, char *buffer, int len);

#endif /* PROXY_HTTP_H */
