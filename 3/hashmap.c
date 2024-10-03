#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "hashmap.h"

void hashmap_init(struct hashmap *map, int cap) {
    map->cap = cap;
    map->entries = calloc(cap, sizeof(struct hashmap_entry));
}

void hashmap_destroy(struct hashmap *map) {
    free(map->entries);
}

void hashmap_insert(
    struct hashmap *map, char *url, struct cache_entry *cache_entry
) {
    hashmap_insert_hash(map, url, hash(url, strlen(url)), cache_entry);
}

void hashmap_insert_hash(
    struct hashmap *map, char *url, uint64_t h, struct cache_entry *cache_entry
) {
    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->url) {
            entry->url = url;
            entry->url_hash = h;
            entry->entry = cache_entry;
            break;
        }
    }
}

void hashmap_get(
    struct hashmap *map, char *url, struct cache_entry **cache_entry
) {
    uint64_t h = hash(url, strlen(url));

    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->url) {
            break;
        }

        if (entry->url_hash == h) {
            if (!strcmp(url, entry->url)) {
                if (cache_entry) {
                    *cache_entry = entry->entry;
                }

                break;
            }
        }
    }
}

void hashmap_remove(
    struct hashmap *map, char *url, struct cache_entry **cache_entry
) {
    uint64_t h = hash(url, strlen(url));

    int idx;

    for (idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->url) {
            return;
        }

        if (entry->url_hash == h) {
            if (!strcmp(url, entry->url)) {
                if (cache_entry) {
                    *cache_entry = entry->entry;
                }

                entry->url = NULL;
                entry->url_hash = 0;
                break;
            }
        }
    }

    for (int jdx = (idx + 1) % map->cap; 1; jdx = (jdx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[jdx];

        if (!entry->url) {
            break;
        }

        char *reinserted_url = entry->url;
        uint64_t reinserted_url_hash = entry->url_hash;
        struct cache_entry *reinserted_cache_entry = entry->entry;

        entry->url = NULL;
        entry->url_hash = 0;

        hashmap_insert_hash(
            map, reinserted_url, reinserted_url_hash, reinserted_cache_entry
        );
    }
}
