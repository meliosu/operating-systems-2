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
