#ifndef ER_UTIL_POOL_H
#define ER_UTIL_POOL_H

#include <stddef.h>

typedef struct er_pool_t er_pool_t;

er_pool_t *er_pool_new(size_t blocksize);

void er_pool_delete(er_pool_t *pool);

void *er_pool_alloc(er_pool_t *pool, size_t size);

void *er_pool_aligned_alloc(er_pool_t *pool, size_t size, size_t align);

#endif
