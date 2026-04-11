#ifndef ER_COMPILER_TRANSFER_H
#define ER_COMPILER_TRANSFER_H

#include "compiler/ir.h"
#include "compiler/type.h"
#include "compiler/build.h"

typedef struct {
    er_type_t *type;
} er_semval_t;

typedef struct {
    er_semval_t *data;
    size_t size;
    size_t cap;
} er_semstate_t;

typedef struct {
    er_buildfunc_t *func;
    er_type_t *rettype;

    er_type_t *Int;
    er_type_t *Bool;
    er_type_t *Error;
} er_semctx_t;

void er_semstate_init(er_semstate_t *state);

void er_semstate_destruct(er_semstate_t *state);

void er_semctx_init(er_semctx_t *ctx, er_buildctx_t *bctx, 
                    er_buildfunc_t *bfunc);

void er_transfer(er_irnode_t *node, er_semstate_t *state, er_semctx_t *ctx);

#endif
