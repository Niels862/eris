#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>

void *er_xmalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "malloc(%zu): %s\n", size, strerror(errno));
        abort();
    }
    return p;
}

void *er_xrealloc(void *p, size_t size) {
    void *p2 = realloc(p, size);
    if (p2 == NULL) {
        fprintf(stderr, "realloc(%zu): %s\n", size, strerror(errno));
        abort();
    }
    return p2;
}

void er_invalidate(void *p, size_t size) {
    uint32_t d = 0xDEADBEEFUL;

    for (size_t i = 0; i < size; i++) {
        ((uint8_t *)p)[i] = ((uint8_t *)&d)[i % sizeof(d)];
    }
}
