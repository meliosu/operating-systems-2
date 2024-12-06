#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "hashmap.h"

static void hashmap_grow(hashmap_t *map) {
    int new_cap = map->cap * 2 ?: 2;

    hashmap_t new = {
        .cap = new_cap,
        .len = 0,
        .entries = calloc(new_cap, sizeof(hashmap_entry_t)),
    };

    for (int i = 0; i < map->cap; i++) {
        hashmap_entry_t *entry = &map->entries[i];

        if (entry->key) {
            hashmap_insert(&new, entry->key, entry->value);
        }
    }

    hashmap_destroy(map);
    *map = new;
}

void hashmap_init(hashmap_t *map) {
    map->len = 0;
    map->cap = 0;
    map->entries = NULL;
}

void hashmap_destroy(hashmap_t *map) {
    if (map->entries) {
        free(map->entries);
    }
}

void hashmap_insert(hashmap_t *map, char *key, void *value) {
    // ensure that map has more capacity the
    if (map->len >= map->cap / 2) {
        hashmap_grow(map);
    }

    uint64_t h = hash(key, strlen(key));

    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        hashmap_entry_t *entry = &map->entries[idx];

        if (!entry->key) {
            entry->key = key;
            entry->value = value;
            break;
        }
    }

    map->len += 1;
}

void hashmap_get(hashmap_t *map, char *key, void **value) {
    if (map->len == 0) {
        *value = NULL;
        return;
    }

    uint64_t h = hash(key, strlen(key));

    // No infinite loop since map->len < map->cap is always satisfied
    // due to grow in hashmap_insert
    for (int idx = h % map->cap; 1; idx = (idx + 1) % map->cap) {
        hashmap_entry_t *entry = &map->entries[idx];

        if (!entry->key) {
            *value = NULL;
            break;
        } else if (!strcmp(entry->key, key)) {
            *value = entry->value;
            break;
        }
    }
}

void hashmap_remove(hashmap_t *map, char *key, void **value) {
    if (map->len == 0) {
        *value = NULL;
        return;
    }

    uint64_t h = hash(key, strlen(key));

    int idx;
    for (idx = h & map->cap; 1; idx = (idx + 1) % map->cap) {
        hashmap_entry_t *entry = &map->entries[idx];

        if (!entry->key) {
            if (value) {
                *value = NULL;
            }

            return;
        } else if (!strcmp(entry->key, key)) {
            if (value) {
                *value = entry->value;
            }

            entry->key = NULL;
            entry->value = NULL;
            break;
        }
    }

    for (int jdx = (idx + 1) % map->cap; 1; jdx = (jdx + 1) % map->cap) {
        hashmap_entry_t *entry = &map->entries[jdx];

        if (!entry->key) {
            break;
        } else {
            char *rekey = entry->key;
            char *revalue = entry->value;

            entry->key = NULL;
            entry->value = NULL;

            hashmap_insert(map, rekey, revalue);
        }
    }

    map->len -= 1;
}
