#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "log.h"

#define BUFSIZE 16384

int log_level = LEVEL_DEFAULT;

void init_log() {
    char *level = getenv("PROXY_LOG");

    if (!level) {
        log_level = LEVEL_DEFAULT;
    } else if (!strcasecmp("ERROR", level)) {
        log_level = LEVEL_ERROR;
    } else if (!strcasecmp("WARN", level)) {
        log_level = LEVEL_WARN;
    } else if (!strcasecmp("INFO", level)) {
        log_level = LEVEL_INFO;
    } else if (!strcasecmp("DEBUG", level)) {
        log_level = LEVEL_DEBUG;
    } else if (!strcasecmp("TRACE", level)) {
        log_level = LEVEL_TRACE;
    } else if (!strcasecmp("OFF", level)) {
        log_level = LEVEL_OFF;
    } else {
        log_level = LEVEL_DEFAULT;
    }
}

void do_log(char *fmt, ...) {
    static char buffer[BUFSIZE];

    va_list args;
    va_start(args, fmt);

    time_t now = time(NULL);

    int len = 0;
    int time_len = 0;

    if (now > 0) {
        struct tm *time = localtime(&now);

        if (time) {
            time_len = strftime(buffer, BUFSIZE, "[%x %X] ", time);
            len += time_len;
        }
    }

    len += vsnprintf(buffer + len, BUFSIZE - len, fmt, args);

    if (len >= BUFSIZE) {
        char *buf = malloc(len + 1);
        memcpy(buf, buffer, time_len);
        vsnprintf(buf + time_len, len + 1 - time_len, fmt, args);
        fprintf(stderr, "%s\n", buf);
        free(buf);
    } else {
        fprintf(stderr, "%s\n", buffer);
    }

    va_end(args);
}
