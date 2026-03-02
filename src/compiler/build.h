#ifndef ER_COMPILER_BUILDMOD_H
#define ER_COMPILER_BUILDMOD_H

#include "util/arena.h"
#include <stddef.h>

typedef struct {
    char *filename;
    char *text;
    size_t size;

    struct {
        er_arena_t *parse;
    } arenas;
} er_buildmod_t;

er_buildmod_t *er_buildmod_read(char const *module);

void er_buildmod_delete(er_buildmod_t *bmod);

#endif
