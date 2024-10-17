#ifndef PROXY_SIEVE_H
#define PROXY_SIEVE_H

#include <pthread.h>
#include <stdatomic.h>

#include "hashmap.h"
#include "stream.h"

typedef struct cache_entry {
    char *key;
    stream_t *value;

    atomic_bool visited;
    struct cache_entry *prev;
    struct cache_entry *next;
} cache_entry_t;

typedef struct cache {
    int len;
    int cap;
    hashmap_t map;
    cache_entry_t *head;
    cache_entry_t *tail;
    cache_entry_t *hand;
    pthread_rwlock_t lock;
} cache_t;

void sieve_cache_init(cache_t *cache, int cap);
void sieve_cache_destroy(cache_t *cache);

void sieve_cache_insert(cache_t *cache, char *key, stream_t *value);
void sieve_cache_lookup(cache_t *cache, char *key, stream_t **value);
void sieve_cache_lookup_or_insert(
    cache_t *cache, char *key, stream_t **looked_up, stream_t *inserted
);

#endif /* PROXY_SIEVE_H */
