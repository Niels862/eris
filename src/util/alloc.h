#ifndef ER_UTIL_ALLOC_H
#define ER_UTIL_ALLOC_H

#include <stddef.h>

void *er_xmalloc(size_t size);

void *er_xrealloc(void *p, size_t size);

void er_invalidate(void *p, size_t size);

#endif
