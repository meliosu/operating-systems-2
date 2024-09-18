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
} node_t;

typedef struct queue_t {
    node_t *head;
} queue_t;

node_t *node_random();

void queue_init(queue_t *queue, int size);
void queue_destroy(queue_t *queue);
