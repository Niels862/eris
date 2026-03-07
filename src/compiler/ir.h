#ifndef ER_COMPILER_IR_H
#define ER_COMPILER_IR_H

#include "util/string.h"
#include <stddef.h>
#include <stdint.h>

typedef uint16_t er_irblockref_t;

typedef uint16_t er_irslotref_t;

#define ER_IRTAGS(X) \
        X(PUSHINT,  S64) \
        X(RET,      NONE) 

#define X(n, k) ER_IR_##n,
typedef enum {
    ER_IRTAGS(X)
} er_irtag_t;
#undef X

typedef enum {
    ER_IRDATA_NONE,
    ER_IRDATA_S64,
} er_irdatakind_t;

typedef struct {
    er_irtag_t tag;
    er_textpos_t pos;
    union {
        int64_t s64;
    } data;
} er_irnode_t;

typedef struct {
    er_irnode_t *nodes;
    size_t n_nodes;
} er_irblock_t;

void er_irnode_print(er_irnode_t *node);

#endif
