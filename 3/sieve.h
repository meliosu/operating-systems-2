#ifndef PROXY_SIEVE_H
#define PROXY_SIEVE_H

#include <stdbool.h>

#include <pthread.h>

#include "hashmap.h"
#include "response.h"
#include "slice.h"

struct cache_entry {
    slice_t request;
    struct response *response;

    bool visited;
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

void sieve_cache_lookup(
    struct cache *cache, slice_t request, struct response **response
);

void sieve_cache_insert(
    struct cache *cache, slice_t request, struct response *response
);

void sieve_cache_get_or_insert(
    struct cache *cache,
    slice_t request,
    struct response **get,
    struct response *insert
);

#endif /* PROXY_SIEVE_H */
