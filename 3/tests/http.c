#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../http.h"

void test_partial_request() {
    int err;

    http_request_t request;
    http_request_init(&request);

    const char *parts[] = {
        "GE",
        "T http://goo",
        "gle.com HTT",
        "P/1.0\r\n\r\n",
    };

    for (int i = 0; i < (sizeof parts / sizeof parts[0]); i++) {
        const char *part = parts[i];
        int len = strlen(part);
        err = http_request_parse(&request, (void *)part, len);
        assert(err == 0);
    }

    assert(request.finished == 1);
    assert(request.version.major == 1 && request.version.minor == 0);
    assert(strcmp("http://google.com", request.url) == 0);
}

void test_partial_response() {
    int err;

    http_response_t response;
    http_response_init(&response);

    /*const char *parts[] = {*/
    /*    "HTT",*/
    /*    "P/1",*/
    /*    ".0",*/
    /*    " 20",*/
    /*    "0 OK\r\n\r\n",*/
    /*};*/

    const char *parts[] = {
        "HTT",
        "P/1",
        ".0 ",
        "20",
        "0 O",
        "K\r\n",
        "Content-",
        "Length: 1\r\n\r\n",
        "a",
    };

    for (int i = 0; i < (sizeof parts / sizeof parts[0]); i++) {
        const char *part = parts[i];
        int len = strlen(part);
        err = http_response_parse(&response, (void *)part, len);
        assert(err == 0);
    }

    assert(response.finished == 1);
    assert(response.version.major == 1 && response.version.minor == 0);
    assert(response.status == 200);
}

int main() {
    test_partial_request();
    test_partial_response();
}
