#ifndef WRAPPERS_H
#define WRAPPERS_H

#include "panic.h"

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

#endif /* WRAPPERS_H */
