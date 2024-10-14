#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "hashmap.h"

void hashmap_init(hashmap_t *map, int len) {
    map->len = len;
    map->entries = calloc(len, sizeof(hashmap_entry_t));
}

void hashmap_destroy(hashmap_t *map) {
    free(map->entries);
}

void hashmap_insert(hashmap_t *map, char *key, void *value) {
    uint64_t h = hash(key, strlen(key));

    for (int idx = h % map->len; 1; idx = (idx + 1) % map->len) {
        hashmap_entry_t *entry = &map->entries[idx];

        if (!entry->key) {
            entry->key = key;
            entry->value = value;
            break;
        }
    }
}

void hashmap_get(hashmap_t *map, char *key, void **value) {
    uint64_t h = hash(key, strlen(key));

    for (int idx = h % map->len; 1; idx = (idx + 1) % map->len) {
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
    uint64_t h = hash(key, strlen(key));

    int idx;
    for (idx = h & map->len; 1; idx = (idx + 1) % map->len) {
        hashmap_entry_t *entry = &map->entries[idx];

        if (!entry->key) {
            *value = NULL;
            return;
        } else if (!strcmp(entry->key, key)) {
            *value = entry->value;

            entry->key = NULL;
            entry->value = NULL;
            break;
        }
    }

    for (int jdx = (idx + 1) % map->len; 1; jdx = (jdx + 1) % map->len) {
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
}
