#include <stdlib.h>

#include "hash.h"
#include "hashmap.h"
#include "slice.h"

void hashmap_init(struct hashmap *map, int cap) {
    map->cap = cap;
    map->entries = calloc(cap, sizeof(struct hashmap_entry));
}

void hashmap_destroy(struct hashmap *map) {
    free(map->entries);
}

void hashmap_insert(
    struct hashmap *map, slice_t request, struct cache_entry *cache_entry
) {
    hashmap_insert_hash(
        map, request, hash(request.ptr, request.len), cache_entry
    );
}

void hashmap_insert_hash(
    struct hashmap *map,
    slice_t request,
    uint64_t h,
    struct cache_entry *cache_entry
) {
    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->request.ptr) {
            entry->request = request;
            entry->request_hash = h;
            entry->entry = cache_entry;
            break;
        }
    }
}

void hashmap_get(
    struct hashmap *map, slice_t request, struct cache_entry **cache_entry
) {
    uint64_t h = hash(request.ptr, request.len);

    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->request.ptr) {
            break;
        }

        if (entry->request_hash == h) {
            if (!slice_cmp(request, entry->request)) {
                if (cache_entry) {
                    *cache_entry = entry->entry;
                }

                break;
            }
        }
    }
}

void hashmap_remove(
    struct hashmap *map, slice_t request, struct cache_entry **cache_entry
) {
    uint64_t h = hash(request.ptr, request.len);

    int idx;

    for (idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[idx];

        if (!entry->request.ptr) {
            return;
        }

        if (entry->request_hash == h) {
            if (!slice_cmp(request, entry->request)) {
                if (cache_entry) {
                    *cache_entry = entry->entry;
                }

                entry->request.ptr = NULL;
                entry->request.len = 0;
                entry->request_hash = 0;
                break;
            }
        }
    }

    for (int jdx = (idx + 1) % map->cap; 1; jdx = (jdx + 1) % map->cap) {
        struct hashmap_entry *entry = &map->entries[jdx];

        if (!entry->request.ptr) {
            break;
        }

        slice_t reinserted_request = entry->request;
        uint64_t reinserted_request_hash = entry->request_hash;
        struct cache_entry *reinserted_cache_entry = entry->entry;

        entry->request.ptr = NULL;
        entry->request.len = 0;
        entry->request_hash = 0;

        hashmap_insert_hash(
            map,
            reinserted_request,
            reinserted_request_hash,
            reinserted_cache_entry
        );
    }
}
