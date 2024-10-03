#ifndef PROXY_HASHMAP_H
#define PROXY_HASHMAP_H

#include <stdint.h>
#include <stdlib.h>

#include "slice.h"

struct hashmap_entry {
    uint64_t url_hash;
    char *url;
    struct cache_entry *entry;
};

struct hashmap {
    int cap;
    struct hashmap_entry *entries;
};

void hashmap_init(struct hashmap *map, int cap);
void hashmap_destroy(struct hashmap *map);
void hashmap_insert(struct hashmap *map, char *url, struct cache_entry *entry);
void hashmap_insert_hash(
    struct hashmap *map, char *url, uint64_t h, struct cache_entry *entry
);
void hashmap_get(struct hashmap *map, char *url, struct cache_entry **entry);
void hashmap_remove(struct hashmap *map, char *url, struct cache_entry **entry);

#endif /* PROXY_HASHMAP_H */
