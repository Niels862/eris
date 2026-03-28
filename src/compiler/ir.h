#ifndef ER_COMPILER_IR_H
#define ER_COMPILER_IR_H

#include "compiler/ast.h"
#include "util/string.h"
#include <stddef.h>
#include <stdint.h>

typedef uint16_t er_irblockref_t;

typedef uint16_t er_irslotref_t;

#define ER_IRTAGS(X) \
        X(PUSHINT,  S64) \
        X(BINOP,    BINOP) \
        X(RET,      NONE) 

#define X(n, k) ER_IR_##n,
typedef enum {
    ER_IRTAGS(X)
} er_irtag_t;
#undef X

typedef enum {
    ER_IRDATA_NONE,
    ER_IRDATA_S64,
    ER_IRDATA_BINOP,
} er_irdatakind_t;

typedef struct {
    er_irtag_t tag;
    er_textpos_t pos;
    union {
        int64_t s64;
        er_binop_t binop;
    } data;
} er_irnode_t;

typedef struct {
    er_irnode_t *nodes;
    size_t n_nodes;
} er_irblock_t;

char const *er_ir_name(er_irtag_t tag);

er_irdatakind_t er_ir_kind(er_irtag_t tag);

void er_irnode_print(er_irnode_t *node);

void er_irblock_print(er_irblock_t *block);

#endif
