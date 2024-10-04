#ifndef PROXY_HASHMAP_H
#define PROXY_HASHMAP_H

struct hashmap_entry {
    char *key;
    void *value;
};

struct hashmap {
    int len;
    struct hashmap_entry *entries;
};

void hashmap_init(struct hashmap *map, int len);
void hashmap_destroy(struct hashmap *map);

void hashmap_insert(struct hashmap *map, char *key, void *value);
void hashmap_get(struct hashmap *map, char *key, void **value);
void hashmap_remove(struct hashmap *map, char *key, void **value);

#endif /* PROXY_HASHMAP_H */
