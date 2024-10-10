#ifndef PANIC_H
#define PANIC_H

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

#define pthread_mutex_init(...) PANIC(pthread_mutex_init, __VA_ARGS__)
#define pthread_cond_init(...) PANIC(pthread_cond_init, __VA_ARGS__)
#define pthread_spin_init(...) PANIC(pthread_spin_init, __VA_ARGS__)
#define pthread_mutex_destroy(...) PANIC(pthread_mutex_destroy, __VA_ARGS__)
#define pthread_cond_destroy(...) PANIC(pthread_cond_destroy, __VA_ARGS__)
#define pthread_spin_destroy(...) PANIC(pthread_spin_destroy, __VA_ARGS__)

#define pthread_mutex_lock(...) PANIC(pthread_mutex_lock, __VA_ARGS__)
#define pthread_mutex_unlock(...) PANIC(pthread_mutex_unlock, __VA_ARGS__)
#define pthread_spin_lock(...) PANIC(pthread_spin_lock, __VA_ARGS__)
#define pthread_spin_unlock(...) PANIC(pthread_spin_unlock, __VA_ARGS__)
#define pthread_cond_wait(...) PANIC(pthread_cond_wait, __VA_ARGS__)
#define pthread_cond_signal(...) PANIC(pthread_cond_signal, __VA_ARGS__)

#define sem_init(...) PANIC_ERRNO(sem_init, __VA_ARGS__)
#define sem_destroy(...) PANIC_ERRNO(sem_destroy, __VA_ARGS__)
#define sem_wait(...) PANIC_ERRNO(sem_wait, __VA_ARGS__)
#define sem_post(...) PANIC_ERRNO(sem_post, __VA_ARGS__)

#endif /* PANIC_H */
