#ifndef PROXY_SLICE_H
#define PROXY_SLICE_H

typedef struct slice_t {
    char *ptr;
    long len;
} slice_t;

int slice_cmp_str(slice_t slice, char *string);

#endif /* PROXY_SLICE_H */
