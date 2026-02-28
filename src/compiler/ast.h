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

typedef union {
    struct {
        er_astnode_t **funcs;
        size_t n_funcs;
    } mod;

    struct {
        er_str_t name;

        er_astnode_t **stmts;
        size_t n_stmts;
    } func;

    struct {
        er_astnode_t *val;
    } s_ret;

    struct {
        int64_t val;
    } e_int;
} er_astdata_t;

struct er_astnode_t {
    er_astkind_t kind;
    er_textpos_t pos;
    er_astdata_t d;
};

void er_ast_print(er_astnode_t *n);

#endif
