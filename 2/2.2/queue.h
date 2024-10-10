#ifndef QUEUE_H
#define QUEUE_H

#define panic(fmt, args...)                                                    \
    do {                                                                       \
        printf(fmt "\n", ##args);                                              \
        exit(1);                                                               \
    } while (0)

typedef struct qnode {
    int value;
    struct qnode *next;
} qnode_t;

typedef struct queue_sync queue_sync_t;

typedef struct queue {
    qnode_t *first;
    qnode_t *last;

    int count;
    int max_count;

    long add_attempts;
    long add_count;
    long get_attempts;
    long get_count;

    queue_sync_t *sync;
} queue_t;

void queue_sync_init(queue_t *queue);
void queue_sync_destroy(queue_t *queue);

void queue_add_lock(queue_t *queue);
void queue_add_unlock(queue_t *queue);

void queue_get_lock(queue_t *queue);
void queue_get_unlock(queue_t *queue);

qnode_t *qnode_create(int value);
void qnode_destroy(qnode_t *qnode);

queue_t *queue_create(int max_count);
void queue_destroy(queue_t *queue);

int queue_add(queue_t *queue, int value);
int queue_get(queue_t *queue, int *value);

void queue_print_stats(queue_t *queue);

#endif /* QUEUE_H */
