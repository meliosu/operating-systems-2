#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "log.h"

#define TIME_BUFFER_SIZE 20

enum log_level log_level = LOG_LEVEL_DEFAULT;

static const char *labels[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG",
    "TRACE",
};

static const char *colors[] = {
    COLOR_RED,
    COLOR_YELLOW,
    COLOR_DEFAULT,
    COLOR_CYAN,
    COLOR_MAGENTA,
};

static void log_gettime(char *buffer, int len) {
    time_t now = time(NULL);
    if (now < 0) {
        buffer[0] = 0;
        return;
    }

    struct tm *tm = localtime(&now);
    if (!tm) {
        buffer[0] = 0;
        return;
    }

    if (!strftime(buffer, len, "[%d/%m/%y %H:%M:%S]", tm)) {
        buffer[0] = 0;
        return;
    }
}

void _log(enum log_level level, const char *fmt, va_list args) {
    char time[TIME_BUFFER_SIZE];
    log_gettime(time, TIME_BUFFER_SIZE);

    int info_len = snprintf(
        NULL, 0, "%s %s%s%s ", time, colors[level], labels[level], COLOR_DEFAULT
    );

    va_list copy;
    va_copy(copy, args);

    int log_len = vsnprintf(NULL, 0, fmt, copy);

    va_end(copy);

    char *buffer = malloc(info_len + log_len + 1);

    snprintf(
        buffer,
        info_len + 1,
        "%s %s%s%s ",
        time,
        colors[level],
        labels[level],
        COLOR_DEFAULT
    );

    vsnprintf(buffer + info_len, log_len + 1, fmt, args);

    fprintf(stderr, "%s\n", buffer);

    free(buffer);
}

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
