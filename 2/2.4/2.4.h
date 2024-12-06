#include <stdatomic.h>

struct spinlock {
    atomic_bool flag;
};

struct spinlock spinlock_create();
void spinlock_lock(struct spinlock *lock);
void spinlock_unlock(struct spinlock *lock);

struct mutex {
    atomic_uint_fast32_t flag;
};

struct mutex mutex_create();
void mutex_lock(struct mutex *mutex);
void mutex_unlock(struct mutex *mutex);
