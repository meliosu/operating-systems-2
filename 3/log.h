#ifndef PROXY_LOG_H
#define PROXY_LOG_H

#include <stdarg.h>
#include <stdio.h>

#define COLOR_DEFAULT "\x1b[39m"
#define COLOR_RED "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"

enum log_level {
    LOG_LEVEL_OFF = -1,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEFAULT = LOG_LEVEL_INFO,
};

extern int log_level;

void log_init_logger();
void _log(enum log_level level, const char *fmt, va_list args);

#define DEFINE_LOG_ALIAS(name, level)                                          \
    __attribute__((format(printf, 1, 2))) static inline void name(             \
        const char *format, ...                                                \
    ) {                                                                        \
        if (log_level >= level) {                                              \
            va_list args;                                                      \
            va_start(args, format);                                            \
            _log(level, format, args);                                         \
            va_end(args);                                                      \
        }                                                                      \
    }

DEFINE_LOG_ALIAS(log_error, LOG_LEVEL_ERROR);
DEFINE_LOG_ALIAS(log_warn, LOG_LEVEL_WARN);
DEFINE_LOG_ALIAS(log_info, LOG_LEVEL_INFO);
DEFINE_LOG_ALIAS(log_debug, LOG_LEVEL_DEBUG);
DEFINE_LOG_ALIAS(log_trace, LOG_LEVEL_TRACE);

#endif /* PROXY_LOG_H */
