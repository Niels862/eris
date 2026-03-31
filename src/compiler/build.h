#ifndef ER_COMPILER_BUILDMOD_H
#define ER_COMPILER_BUILDMOD_H

#include "compiler/token.h"
#include "compiler/ast.h"
#include "compiler/ir.h"
#include "compiler/symbol.h"
#include "compiler/type.h"
#include "module/mod.h"
#include "util/arena.h"
#include "util/error.h"
#include <stddef.h>

typedef struct {
    er_astnode_t *root;
    er_str_t *name;

    struct {
        er_arena_t *ir;
    } arenas;

    er_irblock_t *blocks;
    er_irblock_t *entry;
    size_t n_blocks;

    size_t n_temps;
} er_buildfunc_t;

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

    er_symtab_t globals;

    er_buildfunc_t *bfuncs;
    size_t n_bfuncs;

    er_mod_t *mod;
} er_buildmod_t;

typedef struct {
    struct {
        er_arena_t *persistent;
    } arenas;

    er_typefactory_t tf;
    er_symtab_t builtins;
    
    struct {
        er_sym_t *Int;
    } sym;
} er_buildctx_t;

er_mod_t **er_build(char const *entry);

#endif
