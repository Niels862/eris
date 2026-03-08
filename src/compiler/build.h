#ifndef ER_COMPILER_BUILDMOD_H
#define ER_COMPILER_BUILDMOD_H

#include "compiler/token.h"
#include "compiler/ast.h"
#include "util/arena.h"
#include "util/error.h"
#include <stddef.h>

typedef struct {
    char *filename;
    char *text;
    size_t size;

    struct {
        er_arena_t *persistent;
        er_arena_t *parse;
    } arenas;

    er_tok_t *toks;
    er_astnode_t *root;
} er_buildmod_t;

typedef struct {
    er_astnode_t *root;
} er_buildfunc_t;

er_buildmod_t *er_buildmod_read(char const *module);

void er_buildmod_delete(er_buildmod_t *bmod);

er_buildfunc_t *er_buildfunc_new(er_buildmod_t *bmod, er_astnode_t *funcnode);

#endif
