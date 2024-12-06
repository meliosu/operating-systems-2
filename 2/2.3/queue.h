#include <pthread.h>
#include <stdio.h>

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt, ##args);                                                   \
        abort();                                                               \
    } while (0);

typedef struct node_t {
    char value[100];
    struct node_t *next;

#if defined SYNC_MUTEX
    pthread_mutex_t mutex;

#elif defined SYNC_RWLOCK
    pthread_rwlock_t rwlock;

#elif defined SYNC_SPINLOCK
    pthread_spinlock_t spinlock;

#endif

} node_t;

typedef struct queue_t {
    node_t *head;
} queue_t;

node_t *node_random();

void node_lock_read(node_t *node);
void node_lock_write(node_t *node);
void node_unlock(node_t *node);

void queue_init(queue_t *queue, int size);
void queue_destroy(queue_t *queue);
