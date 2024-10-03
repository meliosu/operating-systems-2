#include "slice.h"

int slice_cmp_str(slice_t slice, char *string) {
    int idx;

    for (idx = 0; idx < slice.len; idx += 1) {
        if (slice.ptr[idx] != string[idx]) {
            return 1;
        }

        if (string[idx] == 0) {
            return 1;
        }
    }

    if (string[idx] != 0) {
        return 1;
    }

    return 0;
}

int slice_cmp(slice_t slice, slice_t other) {
    if (slice.len != other.len) {
        return 1;
    }

    for (int i = 0; i < slice.len; i++) {
        if (slice.ptr[i] != other.ptr[i]) {
            return 1;
        }
    }

    return 0;
}
