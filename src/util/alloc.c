#include "util/alloc.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void *er_xmalloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
        perror("malloc");
        abort();
    }
    return p;
}

void *er_xrealloc(void *p, size_t size) {
    void *p2 = realloc(p, size);
    if (p2 == NULL) {
        perror("realloc");
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
