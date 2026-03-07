#ifndef ER_UTIL_ARENA_H
#define ER_UTIL_ARENA_H

#include <stddef.h>

typedef struct er_arena_t er_arena_t;

er_arena_t *er_arena_new(size_t blocksize);

void er_arena_delete(er_arena_t *arena);

void *er_arena_alloc(er_arena_t *arena, size_t size);

// If LEN < 0 then S should be zero-terminated
char *er_arena_string_alloc(er_arena_t *arena, char const *s, int len);

void *er_arena_aligned_alloc(er_arena_t *arena, size_t size, size_t align);

void *er_arena_realloc(er_arena_t *arena, void *p,
                       size_t nmemb_old, size_t nmemb_new, size_t membsize);

void *er_arena_aligned_realloc(er_arena_t *arena, void *p, size_t align,
                               size_t nmemb_old, size_t nmemb_new, 
                               size_t membsize);

#endif
