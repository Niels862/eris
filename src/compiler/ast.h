#ifndef ER_COMPILER_AST_H
#define ER_COMPILER_AST_H

#include "util/string.h"
#include <stddef.h>

typedef enum {
    ER_AST_NONE,

    ER_AST_MOD,
    ER_AST_FUNC,
    ER_AST_RET,
    ER_AST_INT,
} er_astkind_t;

typedef struct er_astnode_t er_astnode_t;

typedef union {
    struct {
        er_astnode_t **funcs;
        size_t n_funcs;
    } mod;

    struct {
        er_str_t *name;

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
    er_astdata_t data;
};

#endif
