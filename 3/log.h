#ifndef PROXY_LOG_H
#define PROXY_LOG_H

#include <stdio.h>

#define LEVEL_OFF 0
#define LEVEL_ERROR 1
#define LEVEL_WARN 2
#define LEVEL_INFO 3
#define LEVEL_DEBUG 4
#define LEVEL_TRACE 5

#define LEVEL_DEFAULT LEVEL_WARN

#define COLOR_DEFAULT "\x1b[39m"
#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"

extern int log_level;

#define LOG(level, label, color, fmt, args...)                                 \
    do {                                                                       \
        if (log_level >= level) {                                              \
            fprintf(                                                           \
                stderr, "%s " fmt "\n", color label COLOR_DEFAULT, ##args      \
            );                                                                 \
        }                                                                      \
    } while (0);

#define ERROR(args...) LOG(LEVEL_ERROR, "ERROR", COLOR_RED, ##args)
#define WARN(args...) LOG(LEVEL_WARN, "WARN ", COLOR_YELLOW, ##args)
#define INFO(args...) LOG(LEVEL_INFO, "INFO ", COLOR_DEFAULT, ##args)
#define DEBUG(args...) LOG(LEVEL_DEBUG, "DEBUG", COLOR_CYAN, ##args)
#define TRACE(args...) LOG(LEVEL_TRACE, "TRACE", COLOR_MAGENTA, ##args)

void init_log();
void do_log(char *fmt, ...);

#endif /* PROXY_LOG_H */
