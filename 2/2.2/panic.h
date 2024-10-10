#ifndef PANIC_H
#define PANIC_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt "\n", ##args);                                              \
        exit(1);                                                               \
    } while (0)

#define PANIC(function, ...)                                                   \
    do {                                                                       \
        int __err = function(__VA_ARGS__);                                     \
        if (__err) {                                                           \
            panic(                                                             \
                "%s:%d " #function ": %s", __FILE__, __LINE__, strerror(__err) \
            );                                                                 \
        }                                                                      \
    } while (0)

#define PANIC_ERRNO(function, ...)                                             \
    do {                                                                       \
        int __err = function(__VA_ARGS__);                                     \
        if (__err < 0) {                                                       \
            panic(                                                             \
                "%s:%d " #function ": %s", __FILE__, __LINE__, strerror(errno) \
            );                                                                 \
        }                                                                      \
    } while (0)

#endif /* PANIC_H */
