#include <pthread.h>
#include <string.h>

#include "hashmap.h"
#include "response.h"
#include "sieve.h"

void sieve_cache_init(struct cache *cache, int cap) {
    memset(cache, 0, sizeof(*cache));
    cache->cap = cap;
    hashmap_init(&cache->map, cap * 2);
    pthread_rwlock_init(&cache->lock, NULL);
}

void sieve_cache_destroy(struct cache *cache) {
    pthread_rwlock_destroy(&cache->lock);
    hashmap_destroy(&cache->map);
}

void sieve_cache_lookup(
    struct cache *cache, slice_t request, struct response **response
) {
    pthread_rwlock_rdlock(&cache->lock);

    struct cache_entry *cache_entry;
    hashmap_get(&cache->map, request, &cache_entry);

    if (cache_entry) {
        *response = cache_entry->response;
        bool visited = 1;
        __atomic_store(&cache_entry->visited, &visited, __ATOMIC_RELAXED);
    } else {
        *response = NULL;
    }

    pthread_rwlock_unlock(&cache->lock);
}

static void sieve_cache_evict(struct cache *cache) {
    struct cache_entry *curr = cache->hand;

    while (curr) {
        if (!curr->visited) {
            if (curr == cache->tail) {
                curr->next->prev = NULL;
                cache->tail = curr->next;
                cache->hand = curr->next;
            } else if (curr == cache->head) {
                curr->prev->next = NULL;
                cache->head = curr->prev;
                cache->hand = cache->tail;
            } else {
                curr->next->prev = curr->prev;
                curr->prev->next = curr->next;
                cache->hand = curr->next;
            }

            break;
        }

        curr->visited = 0;

        if (!curr->next) {
            curr = cache->tail;
        } else {
            curr = curr->next;
        }
    }

    hashmap_remove(&cache->map, curr->request, NULL);
    free(curr);
}

static void sieve_cache_insert_nonfull(
    struct cache *cache, slice_t request, struct cache_entry *entry
) {
    if (!cache->head) {
        entry->prev = entry->next = NULL;
        cache->head = cache->tail = cache->hand = entry;
    } else {
        cache->head->next = entry;
        entry->prev = cache->head;
        cache->head = entry;
    }

    hashmap_insert(&cache->map, request, entry);

    cache->len += 1;
}

void sieve_cache_insert(
    struct cache *cache, slice_t request, struct response *response
) {
    struct cache_entry *entry = malloc(sizeof(*entry));

    entry->next = entry->prev = NULL;
    entry->request = request;
    entry->response = response;

    pthread_rwlock_wrlock(&cache->lock);

    if (cache->len == cache->cap) {
        sieve_cache_evict(cache);
    }

    sieve_cache_insert_nonfull(cache, request, entry);

    pthread_rwlock_unlock(&cache->lock);
}

void sieve_cache_get_or_insert(
    struct cache *cache,
    slice_t request,
    struct response **get,
    struct response *insert
) {
    *get = NULL;

    sieve_cache_lookup(cache, request, get);

    if (*get) {
        return;
    }

    struct cache_entry *entry = malloc(sizeof(*entry));

    entry->next = entry->prev = NULL;
    entry->request = request;
    entry->response = insert;

    pthread_rwlock_wrlock(&cache->lock);

    struct cache_entry *present_entry = NULL;
    hashmap_get(&cache->map, request, &present_entry);

    if (present_entry) {
        *get = present_entry->response;
        present_entry->visited = true;
        pthread_rwlock_unlock(&cache->lock);
        return;
    }

    if (cache->len == cache->cap) {
        sieve_cache_evict(cache);
    }

    sieve_cache_insert_nonfull(cache, request, entry);

    pthread_rwlock_unlock(&cache->lock);
}
