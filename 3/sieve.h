#ifndef PROXY_SIEVE_H
#define PROXY_SIEVE_H

#include <pthread.h>
#include <stdatomic.h>

#include "hashmap.h"

struct cache_entry {
    char *key;
    void *value;

    atomic_bool visited;
    struct cache_entry *prev;
    struct cache_entry *next;
};

struct cache {
    int len;
    int cap;
    struct hashmap map;
    struct cache_entry *head;
    struct cache_entry *tail;
    struct cache_entry *hand;
    pthread_rwlock_t lock;
};

void sieve_cache_init(struct cache *cache, int cap);
void sieve_cache_destroy(struct cache *cache);

void sieve_cache_insert(struct cache *cache, char *key, void *value);
void sieve_cache_lookup(struct cache *cache, char *key, void **value);
void sieve_cache_lookup_or_insert(
    struct cache *cache, char *key, void **looked_up, void *inserted
);

#endif /* PROXY_SIEVE_H */
