#ifndef PROXY_HASHMAP_H
#define PROXY_HASHMAP_H

typedef struct hashmap_entry {
    char *key;
    void *value;
} hashmap_entry_t;

typedef struct hashmap {
    int len;
    hashmap_entry_t *entries;
} hashmap_t;

void hashmap_init(hashmap_t *map, int len);
void hashmap_destroy(hashmap_t *map);

void hashmap_insert(hashmap_t *map, char *key, void *value);
void hashmap_get(hashmap_t *map, char *key, void **value);
void hashmap_remove(hashmap_t *map, char *key, void **value);

#endif /* PROXY_HASHMAP_H */
