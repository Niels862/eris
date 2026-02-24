#ifndef ER_COMPILER_BUILDMOD_H
#define ER_COMPILER_BUILDMOD_H

#include "util/pool.h"
#include <stddef.h>

typedef struct {
    char const *filename;
    char *text;
    size_t size;

    er_pool_t *pool;
} er_buildmod_t;

er_buildmod_t *er_buildmod_read(char const *module);

void er_buildmod_delete(er_buildmod_t *bmod);

#endif
