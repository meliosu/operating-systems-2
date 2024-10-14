#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "log.h"

#define FORMAT_SIZE 20

int log_level = LOG_LEVEL_DEFAULT;

void log_init_logger() {
    char *level = getenv("PROXY_LOG");

    if (level) {
        if (!strcasecmp("ERROR", level)) {
            log_level = LOG_LEVEL_ERROR;
        } else if (!strcasecmp("WARN", level)) {
            log_level = LOG_LEVEL_WARN;
        } else if (!strcasecmp("INFO", level)) {
            log_level = LOG_LEVEL_INFO;
        } else if (!strcasecmp("DEBUG", level)) {
            log_level = LOG_LEVEL_DEBUG;
        } else if (!strcasecmp("TRACE", level)) {
            log_level = LOG_LEVEL_TRACE;
        } else if (!strcasecmp("OFF", level)) {
            log_level = LOG_LEVEL_OFF;
        }
    }
}

char *log_gettime() {
    thread_local static char buffer[FORMAT_SIZE];

    time_t now;
    if (time(&now) < 0) {
        buffer[0] = 0;
        return buffer;
    }

    struct tm *tm;
    if (!(tm = localtime(&now))) {
        buffer[0] = 0;
        return buffer;
    }

    if (!strftime(buffer, FORMAT_SIZE, "[%d/%m/%y %H:%M:%S]", tm)) {
        buffer[0] = 0;
        return buffer;
    }

    return buffer;
}
