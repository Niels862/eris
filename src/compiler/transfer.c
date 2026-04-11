#include "compiler/transfer.h"
#include "util/list.h"
#include "util/error.h"
#include <stdlib.h>

static void er_push(er_semstate_t *state, er_type_t *type) {
    er_semval_t val = {
        .type = type
    };
    ER_LIST_ADD(state, val);
}

static er_type_t *er_pop(er_semstate_t *state) {
    return state->data[--state->size].type;
}

void er_semstate_init(er_semstate_t *state) {
    ER_LIST_INIT(state, 16);
}

void er_semstate_destruct(er_semstate_t *state) {
    free(state->data);
}

void er_semctx_init(er_semctx_t *ctx, er_buildctx_t *bctx, 
                    er_buildfunc_t *bfunc) {
    ctx->func = bfunc;
    ctx->rettype = bfunc->sym->data.Func.type->data.Func.rettype;

    ctx->Int = bctx->syms.Int->data.Class.type;
    ctx->Bool = bctx->syms.Bool->data.Class.type;
    ctx->Error = bctx->syms.Error->data.Class.type;
}

void er_transfer(er_irnode_t *node, er_semstate_t *state, er_semctx_t *ctx) {
    ER_UNUSED(node), ER_UNUSED(state), ER_UNUSED(ctx);

    switch (node->tag) {
        case ER_IR_PUSHINT: {
            er_push(state, ctx->Int);
            break;
        }

        case ER_IR_BINOP: {
            er_type_t *L = er_pop(state);
            er_type_t *R = er_pop(state);

            if (L == ctx->Int && R == ctx->Int) {
                er_push(state, ctx->Int);
            } else {
                er_push(state, ctx->Error);
            }

            break;
        }

        case ER_IR_RET: {
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", node->tag);
    }
}
