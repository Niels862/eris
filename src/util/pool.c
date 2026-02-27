#include "util/pool.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef struct er_poolblock_t er_poolblock_t;

struct er_pool_t {
    size_t blocksize;
    er_poolblock_t *block;
};

struct er_poolblock_t {
    char *data;
    size_t size;
    size_t at;
    er_poolblock_t *next;
};

static er_poolblock_t *er_poolblock_new(size_t size, er_poolblock_t *next) {
    er_poolblock_t *block = er_xmalloc(sizeof(er_poolblock_t));

    block->data = er_xmalloc(size);
    block->size = size;
    block->at = 0;
    block->next = next;

    er_invalidate(block->data, block->size);

    return block;
}

static void *er_poolblock_aligned_alloc(er_poolblock_t *block, 
                                        size_t size, size_t align) {
    size_t at = block->at;
    at += align - at % align;

    if (at + size > block->size) {
        return NULL;
    }

    block->at = at + size;
    return block->data + at;
}

er_pool_t *er_pool_new(size_t blocksize) {
    er_pool_t *pool = er_xmalloc(sizeof(er_pool_t));

    pool->blocksize = blocksize;
    pool->block = er_poolblock_new(blocksize, NULL);

    return pool;
}

void er_pool_delete(er_pool_t *pool) {
    er_poolblock_t *block = pool->block;
    while (block != NULL) {
        er_poolblock_t *next = block->next;
        free(block->data);
        free(block);
        block = next;
    }

    free(pool);
}

void *er_pool_alloc(er_pool_t *pool, size_t size) {
    return er_pool_aligned_alloc(pool, size, sizeof(void *));
}

char *er_pool_string_alloc(er_pool_t *pool, char const *s, int len) {
    size_t size;
    if (len < 0) {
        size = strlen(s) + 1;
    } else {
        size = len + 1;
    }

    char *s1 = er_pool_aligned_alloc(pool, size, 1);
    memcpy(s1, s, size);
    return s1;
}

void *er_pool_aligned_alloc(er_pool_t *pool, size_t size, size_t align) {
    void *p = er_poolblock_aligned_alloc(pool->block, size, align);
    if (p != NULL) {
        goto end;
    }

    size_t blocksize = pool->blocksize;
    while (blocksize < size) {
        blocksize *= 2;
    }

    pool->block = er_poolblock_new(blocksize, pool->block);
    p = er_poolblock_aligned_alloc(pool->block, size, align);
    
end:
    assert(p != NULL);
    assert((uintptr_t)p % align == 0);
    return p;
}
