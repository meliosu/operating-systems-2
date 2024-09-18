#include <immintrin.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "2.4.h"

struct spinlock spinlock_create() {
    return (struct spinlock){
        .flag = 0,
    };
}

void spinlock_lock(struct spinlock *lock) {
    while (atomic_exchange_explicit(&lock->flag, 1, memory_order_acquire)) {
        continue;
    }
}

void spinlock_unlock(struct spinlock *lock) {
    atomic_store_explicit(&lock->flag, 0, memory_order_release);
}

struct mutex mutex_create() {
    return (struct mutex){
        .flag = 0,
    };
}

int futex_wait(uint32_t *word, uint32_t val, const struct timespec *timeout) {
    return syscall(SYS_futex, word, FUTEX_WAIT, val, timeout);
}

int futex_wake(uint32_t *word, uint32_t num) {
    return syscall(SYS_futex, word, FUTEX_WAKE, num);
}

void mutex_lock(struct mutex *mutex) {
    const int SPINS = 100;

    while (1) {
        for (int i = 0; i < SPINS; i++) {
            if (!atomic_exchange_explicit(
                    &mutex->flag, 1, memory_order_acquire
                )) {
                return;
            }
        }

        futex_wait((uint32_t *)&mutex->flag, 1, NULL);
    }
}

void mutex_unlock(struct mutex *mutex) {
    atomic_store_explicit(&mutex->flag, 0, memory_order_release);
    futex_wake((uint32_t *)&mutex->flag, 1);
}
