#include <stdint.h>

#include "hash.h"

typedef uint64_t u64;
typedef uint8_t u8;

u64 hash(void *key, int len) {
    const u64 seed = 0x485f9ef2b97fd52dULL;
    const u64 m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    u64 h = seed ^ (len * m);

    const u64 *data = (u64 *)key;
    const u64 *end = data + (len / 8);

    while (data != end) {
        u64 k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    u8 *left = (u8 *)data;

    switch (len % 8) {
    case 7:
        h ^= (u64)left[6] << 48;
    case 6:
        h ^= (u64)left[5] << 40;
    case 5:
        h ^= (u64)left[4] << 32;
    case 4:
        h ^= (u64)left[3] << 24;
    case 3:
        h ^= (u64)left[2] << 16;
    case 2:
        h ^= (u64)left[1] << 8;
    case 1:
        h ^= (u64)left[0];
    }

    h *= m;
    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}
