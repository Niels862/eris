#ifndef ER_COMPILER_AST_H
#define ER_COMPILER_AST_H

#include "util/string.h"
#include <stddef.h>

#define ER_AST_NODES(X) \
    X(NONE) \
    X(MOD) \
    X(FUNC) \
    X(RET) \
    X(INT)

#define X(n) ER_AST_##n,
typedef enum {
    ER_AST_NODES(X)
} er_astkind_t;
#undef X

typedef struct er_astnode_t er_astnode_t;

typedef struct {
    er_astnode_t **funcs;
    size_t n_funcs;
} er_astmod_t;

typedef struct {
    er_str_t name;
    er_astnode_t **stmts;
    size_t n_stmts;
} er_astfunc_t;

typedef struct {
    er_astnode_t *val;
} er_astret_t;

typedef struct {
    int64_t val;
} er_astint_t;

typedef union {
    er_astmod_t Mod;
    er_astfunc_t Func;
    er_astret_t Ret;
    er_astint_t Int;
} er_astdata_t;

struct er_astnode_t {
    er_astkind_t kind;
    er_textpos_t pos;
    er_astdata_t data;
};

void er_ast_print(er_astnode_t *n);

#endif
