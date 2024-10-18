#ifndef PROXY_LOG_H
#define PROXY_LOG_H

#include <stdio.h>

#define LOG_LEVEL_OFF 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

#define LOG_LEVEL_DEFAULT LOG_LEVEL_WARN

#define COLOR_DEFAULT "\x1b[39m"
#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"

#define LOG(level, label, color, fmt, args...)                                 \
    do {                                                                       \
        if (log_level >= level) {                                              \
            fprintf(                                                           \
                stderr,                                                        \
                "%s %s " fmt "\n",                                             \
                log_gettime(),                                                 \
                color label COLOR_DEFAULT,                                     \
                ##args                                                         \
            );                                                                 \
        }                                                                      \
    } while (0);

#define ERROR(args...) LOG(LOG_LEVEL_ERROR, "ERROR", COLOR_RED, ##args)
#define WARN(args...) LOG(LOG_LEVEL_WARN, "WARN ", COLOR_YELLOW, ##args)
#define INFO(args...) LOG(LOG_LEVEL_INFO, "INFO ", COLOR_DEFAULT, ##args)
#define DEBUG(args...) LOG(LOG_LEVEL_DEBUG, "DEBUG", COLOR_CYAN, ##args)
#define TRACE(args...) LOG(LOG_LEVEL_TRACE, "TRACE", COLOR_MAGENTA, ##args)

extern int log_level;

void log_init_logger();
char *log_gettime();

#endif /* PROXY_LOG_H */
