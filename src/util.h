#ifndef ER_UTIL_H
#define ER_UTIL_H

#include <stddef.h>

#define ER_UNUSED(v) (void)v
#define ER_MAYBE_UNUSED(v) (void)v

void *er_xmalloc(size_t size);

void *er_xrealloc(void *p, size_t size);

#endif
