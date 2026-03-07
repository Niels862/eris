#include "util/arena.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef struct er_arenablock_t er_arenablock_t;

struct er_arena_t {
    size_t blocksize;
    er_arenablock_t *block;
};

struct er_arenablock_t {
    char *data;
    size_t size;
    size_t at;
    er_arenablock_t *next;
};

static er_arenablock_t *er_arenablock_new(size_t size, er_arenablock_t *next) {
    er_arenablock_t *block = er_xmalloc(sizeof(er_arenablock_t));

    block->data = er_xmalloc(size);
    block->size = size;
    block->at = 0;
    block->next = next;

    er_invalidate(block->data, block->size);

    return block;
}

static void *er_arenablock_aligned_alloc(er_arenablock_t *block, 
                                         size_t size, size_t align) {
    size_t at = block->at;
    at += align - at % align;

    if (at + size > block->size) {
        return NULL;
    }

    block->at = at + size;
    return block->data + at;
}

er_arena_t *er_arena_new(size_t blocksize) {
    er_arena_t *arena = er_xmalloc(sizeof(er_arena_t));

    arena->blocksize = blocksize;
    arena->block = er_arenablock_new(blocksize, NULL);

    return arena;
}

void er_arena_delete(er_arena_t *arena) {
    er_arenablock_t *block = arena->block;
    while (block != NULL) {
        er_arenablock_t *next = block->next;
        free(block->data);
        free(block);
        block = next;
    }

    free(arena);
}

void *er_arena_alloc(er_arena_t *arena, size_t size) {
    return er_arena_aligned_alloc(arena, size, sizeof(void *));
}

char *er_arena_string_alloc(er_arena_t *arena, char const *s, int len) {
    size_t size;
    if (len < 0) {
        size = strlen(s) + 1;
    } else {
        size = len + 1;
    }

    char *s1 = er_arena_aligned_alloc(arena, size, 1);
    memcpy(s1, s, size);
    return s1;
}

void *er_arena_aligned_alloc(er_arena_t *arena, size_t size, size_t align) {
    assert(align == 1 || align == 2 || align == 4 || align == 8);

    void *p = er_arenablock_aligned_alloc(arena->block, size, align);
    if (p != NULL) {
        goto end;
    }

    size_t blocksize = arena->blocksize;
    while (blocksize < size) {
        blocksize *= 2;
    }

    arena->block = er_arenablock_new(blocksize, arena->block);
    p = er_arenablock_aligned_alloc(arena->block, size, align);
    
end:
    assert(p != NULL);
    assert((uintptr_t)p % align == 0);
    return p;
}

void *er_arena_realloc(er_arena_t *arena, void *p,
                       size_t nmemb_old, size_t nmemb_new, size_t membsize) {
    return er_arena_aligned_realloc(arena, p, sizeof(void *), 
                                    nmemb_old, nmemb_new, membsize);
}

void *er_arena_aligned_realloc(er_arena_t *arena, void *p, size_t align,
                       size_t nmemb_old, size_t nmemb_new, size_t membsize) {
    assert(nmemb_old <= nmemb_new);
    assert(nmemb_old > 0);
    
    void *p2 = er_arena_aligned_alloc(arena, nmemb_new * membsize, align);
    memcpy(p2, p, nmemb_old * membsize);
    er_invalidate(p, nmemb_old * membsize);

    return p2;
}
