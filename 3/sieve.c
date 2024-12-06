#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>

#include "hashmap.h"
#include "sieve.h"
#include "stream.h"

void sieve_cache_init(cache_t *cache, int cap) {
    hashmap_init(&cache->map);
    pthread_rwlock_init(&cache->lock, NULL);

    cache->len = 0;
    cache->cap = cap;
    cache->head = NULL;
    cache->tail = NULL;
    cache->hand = NULL;
}

void sieve_cache_destroy(cache_t *cache) {
    hashmap_destroy(&cache->map);
    pthread_rwlock_destroy(&cache->lock);

    cache_entry_t *entry = cache->tail;
    while (entry) {
        cache_entry_t *next = entry->next;
        stream_destroy(entry->value);
        free(entry->key);
        free(entry);
        entry = next;
    }
}

void sieve_cache_lookup(cache_t *cache, char *key, stream_t **value) {
    pthread_rwlock_rdlock(&cache->lock);

    cache_entry_t *cache_entry;
    hashmap_get(&cache->map, key, (void **)&cache_entry);

    if (cache_entry) {
        atomic_store(&cache_entry->visited, true);
        *value = stream_clone(cache_entry->value);
    } else {
        *value = NULL;
    }

    pthread_rwlock_unlock(&cache->lock);
}

static void cache_evict(cache_t *cache) {
    cache_entry_t *curr = cache->hand;

    while (1) {
        if (!curr->visited) {
            if (curr == cache->head) {
                curr->prev->next = NULL;
                cache->head = curr->prev;
                cache->hand = cache->tail;
            } else if (curr == cache->tail) {
                curr->next->prev = NULL;
                cache->tail = curr->next;
                cache->hand = curr->next;
            } else {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;
                cache->hand = curr->next;
            }

            break;
        } else {
            curr->visited = false;
        }

        if (curr->next) {
            curr = curr->next;
        } else {
            curr = cache->tail;
        }
    }

    hashmap_remove(&cache->map, curr->key, NULL);
    stream_destroy(curr->value);
    free(curr);

    cache->len -= 1;
}

static void cache_insert_nonfull(cache_t *cache, char *key, stream_t *value) {
    cache_entry_t *cache_entry = malloc(sizeof(cache_entry_t));

    cache_entry->key = key;
    cache_entry->value = value;
    cache_entry->visited = false;
    cache_entry->prev = NULL;
    cache_entry->next = NULL;

    if (!cache->head) {
        cache->head = cache->tail = cache->hand = cache_entry;
    } else {
        cache->head = cache->head->next = cache_entry;
    }

    hashmap_insert(&cache->map, key, cache_entry);
    cache->len += 1;
}

void sieve_cache_insert(cache_t *cache, char *key, stream_t *value) {
    pthread_rwlock_wrlock(&cache->lock);

    if (cache->len == cache->cap) {
        cache_evict(cache);
    }

    cache_insert_nonfull(cache, key, value);

    pthread_rwlock_unlock(&cache->lock);
}

void sieve_cache_lookup_or_insert(
    cache_t *cache, char *key, stream_t **looked_up, stream_t *inserted
) {
    pthread_rwlock_wrlock(&cache->lock);

    cache_entry_t *cache_entry;
    hashmap_get(&cache->map, key, (void **)&cache_entry);

    if (cache_entry) {
        atomic_store(&cache_entry->visited, true);
        *looked_up = stream_clone(cache_entry->value);
    } else {
        if (cache->len == cache->cap) {
            cache_evict(cache);
        }

        cache_insert_nonfull(cache, key, stream_clone(inserted));
        *looked_up = NULL;
    }

    pthread_rwlock_unlock(&cache->lock);
}
