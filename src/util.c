#include "util.h"
#include <stdlib.h>
#include <stdio.h>

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
